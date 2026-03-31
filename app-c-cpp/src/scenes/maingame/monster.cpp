// monster.cpp
// Ported from BasalMonster.java — preserves state machine, timings, HP scaling,
// special effects, projectile, blow-back, freeze/shock/burn, boss behaviours.

#include "scenes/main_game_scene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "engine/abstract_game.hpp"
#include "engine/sprite_atlas_cache.hpp"
#include "game/achv_mng.hpp"
#include "game/param.hpp"

namespace defender {
namespace {

using MonsterStatus = MainGameScene::MonsterStatus;
constexpr int kEffectCritical = 1;

constexpr int kProjectileKindDevilFire = 1;
constexpr int kProjectileKindBossFire  = 2;
constexpr int kProjectileKindStone     = 3;

// Frame timing constants (ms per frame)
constexpr int kFrameMs          = 125;
constexpr int kStoneFrameMs     = 200;
constexpr int kBoss1JumpFrameMs = 50;

// Die display time (ms)
constexpr int kDieShowTime = 2000;
constexpr int kBeHitTime   = 200;

// Blow-back physics: minimum scaled speed
constexpr float kBlowMinSpeed = 40.0f;

struct MonsterConfig {
    int base_hp = 0;
    int atk = 0;
    int atk_cd_ms = 0;
    float run_speed_units = 0.0f;
    int run_frame_num = 0;
    int atk_frame_num = 0;
    int atk2_frame_num = 0;
    int die_frame_num = 0;
    int jump_frame_num = 0;
    int atk_judge_frame = 0;
    int atk_judge_frame2 = 0;
    int remote_missile_frames = 0;
    int missile_blast_frames = 0;
    bool is_remote_atk = false;
    const char* size_frame_key = nullptr;
};

constexpr std::array<MonsterConfig, 7> kMonsterConfigs{{
    {50,   3, 1000, 100.0f, 6, 10,  0,  3,  0, 7,  0, 0,  0, false, "quanjiniao_run_lan_0001.png"},
    {45,   3,  800, 150.0f, 6,  6,  0,  7,  0, 4,  0, 0,  0, false, "manzutu_run_hong_0001.png"},
    {80,   4, 1200,  80.0f, 8, 12,  0, 10,  0, 6,  0, 0,  0, false, "duyanguai_run_huang_0001.png"},
    {55,   3, 2000, 100.0f, 6, 10,  0, 12,  0, 5,  0, 6,  6, true,  "xiaoemo_run_zi_0001.png"},
    {1400, 7,  500, 240.0f, 6, 12,  0,  6, 10, 4, 11, 0,  0, false, "yemanshouren_run_001.png"},
    {4500, 4,  500, 240.0f, 6, 10, 12,  9,  0, 7,  0, 0,  0, false, "jielingqishi_run_1.png"},
    {450, 15, 4000,  80.0f, 3,  6,  6,  4,  0, 2,  0, 5, 10, true,  "zz_toushiche_run_0001.png"},
}};

const MonsterConfig& monster_config_for(int type) {
    static const MonsterConfig kFallback = kMonsterConfigs[0];
    if (type < 0 || type >= static_cast<int>(kMonsterConfigs.size())) {
        return kFallback;
    }
    return kMonsterConfigs[static_cast<size_t>(type)];
}

float monster_frame_size(const char* frame_key, MainGameScene* scene, bool width) {
    const AtlasFrame* frame = (frame_key == nullptr) ? nullptr : SpriteAtlasCache::instance().get_frame(frame_key);
    if (!frame || !frame->valid) {
        return scene->get_xy(96.0f);
    }
    const int axis = width ? frame->original_width : frame->original_height;
    return scene->get_xy(static_cast<float>(std::max(1, axis)));
}

// -----------------------------------------------------------------------
// HP / speed scaling helpers
// -----------------------------------------------------------------------
static float compute_stage_hp_scale() {
    float temp2 = (Param::stage > 100) ? (1.1f * 1.1f) : 1.1f;
    if (Param::stage > 110) temp2 *= 1.05f;
    if (Param::stage > 150) temp2 *= 1.1f;
    if (Param::stage > 190) temp2 *= 1.1f + ((Param::stage - 190) / 450.0f);
    if (Param::stage > 200) temp2 *= 1.05f;
    return temp2;
}

static float compute_boss_fix() {
    float bossFix = 1.0f;
    if (Param::stage == 10)  bossFix = 0.95f;
    if (Param::stage == 40)  bossFix = 1.05f;
    if (Param::stage == 60)  bossFix = 0.9f;
    if (Param::stage == 150) bossFix = 0.95f;
    if (Param::stage == 190) bossFix = 1.1f;
    if (Param::stage > 190)  bossFix = 1.05f;
    if (Param::stage > 300)  bossFix = 1.15f;
    if (Param::stage > 400)  bossFix = 1.25f;
    return bossFix;
}

static int compute_monster_hp(int base_hp, int type_id) {
    int tempStage  = Param::stage;
    int deltaStage = 0;
    if (tempStage > 400) {
        deltaStage = tempStage - 400;
        tempStage  = 400;
    }
    float temp2 = compute_stage_hp_scale();
    int hp = static_cast<int>((1.0f + (tempStage / 25.0f) + (deltaStage / 100.0f)) * temp2 * base_hp);
    if (type_id == 4) {
        hp = static_cast<int>(hp * compute_boss_fix());
    }
    return hp;
}

static float compute_monster_speed(float base_speed_units, MainGameScene* scene) {
    int tempStage  = Param::stage;
    int deltaStage = 0;
    if (tempStage > 400) {
        deltaStage = tempStage - 400;
        tempStage  = 400;
    }
    float runSpeed = (((tempStage + 400) + (deltaStage / 10)) * 0.85f * base_speed_units) / 400.0f;
    return scene->get_x(runSpeed);
}

static float attack_x_for_type(int type, MainGameScene* scene) {
    switch (type) {
    case 0: return scene->get_x(165.0f);
    case 1: return scene->get_x(170.0f);
    case 2: return scene->get_x(165.0f);
    case 3: return scene->get_x(550.0f);
    case 4: return scene->get_x(180.0f);
    case 5: return scene->get_x(180.0f);
    case 6: return scene->get_x(650.0f);
    default: return scene->get_x(165.0f);
    }
}

static float magic_res_for_type(int type) {
    switch (type) {
    case 4: return 2.0f;
    case 5: return 0.5f;
    case 6: return 5.0f;
    default: return 1.0f;
    }
}

} // namespace

// -----------------------------------------------------------------------
// kill_monster
// -----------------------------------------------------------------------
void MainGameScene::kill_monster(size_t idx) {
    if (idx >= monsters_.size()) return;
    monsters_.erase(monsters_.begin() + static_cast<std::ptrdiff_t>(idx));
}

bool MainGameScene::monster_be_hit(size_t idx, int hit_point, int hit_type) {
    if (idx >= monsters_.size()) {
        return false;
    }

    auto& m = monsters_[idx];
    if (m.type == 4 && m.jump_time_ms > 0) {
        return false;
    }
    if (m.status == MonsterStatus::kDying || m.status == MonsterStatus::kWaiting) {
        return false;
    }

    if (m.type == 4) {
        if (m.status == MonsterStatus::kRunning && !Param::is_online_mode && m.x > get_x(266.0f)) {
            float evade_rate = (Param::stage - 10) * 0.5f;
            if (evade_rate > 20.0f) {
                evade_rate = 20.0f;
            }
            if (evade_rate > 0.0f &&
                static_cast<int>(boss1_rng_() % 100) < static_cast<int>(evade_rate)) {
                m.target_dx = 0.0f;
                m.target_dy = get_y(static_cast<float>(boss1_rng_() % 330 + 90)) - m.y;
                m.img_time_ms = 0;
                m.jump_time_ms = 500;
                return false;
            }
        }
        if (hit_type == 1) {
            static constexpr float kBossY[] = {120.0f, 270.0f, 420.0f};
            m.img_id = m.atk_frame_num + m.run_frame_num + m.die_frame_num;
            m.jump_time_ms = 500;
            m.target_dy = get_y(kBossY[boss1_rng_() % 3]) - m.y;
            m.target_dx = get_x(800.0f) - m.x;
            m.status = MonsterStatus::kRunning;
            m.img_time_ms = 0;
        }
    } else if (m.type == 5 &&
               m.status == MonsterStatus::kAttacking &&
               (m.img_alpha < 1.0f || m.fire_flag) &&
               hit_type == 1) {
        m.disturbDis = 50.0f;
        m.fire_flag = false;
        m.status = MonsterStatus::kRunning;
        m.run_time_ms = 0;
        m.img_time_ms = 0;
        m.target_dx = get_x(static_cast<float>(boss2_rng_() % 400 + 200)) - m.x;
        m.target_dy = get_y(static_cast<float>(boss2_rng_() % 300 + 100)) - m.y;
        m.img_alpha = 1.0f;
    }

    if (hit_type == 2) {
        if (Param::fatal_blow_rate > 0 &&
            static_cast<int>(form_rng_() % 100) < Param::fatal_blow_rate) {
            hit_point *= 2;
            MainGameScene::EffectEntity fx;
            fx.x = m.x;
            fx.y = m.y;
            fx.ttl_ms = 600.0f;
            fx.kind = kEffectCritical;
            fx.w = get_x(65.0f);
            fx.h = get_y(32.0f);
            fx.vx = get_x(32.5f);
            fx.vy = get_y(80.0f);
            effects_.push_back(fx);
        }
        if (Param::atk_spd_dec_rate > 0) {
            const float rate = (100.0f - Param::atk_spd_dec_rate) / 100.0f;
            if (rate > 0.0f) {
                m.atk_cd_ms = static_cast<int>(m.atk_cd_ms * (1.0f / rate));
            }
        }
        if (Param::power_shot_dis > 0) {
            m.blow_dis = get_x(static_cast<float>(Param::power_shot_dis));
        }
        hit_point = static_cast<int>(hit_point * (Param::multi_power / 100.0f));
        m.blow_dis = (Param::multi_power < 100 ? 0.3f : 1.0f) * m.blow_dis;
        if (m.type == 4) {
            m.blow_dis *= 0.25f;
        }
        if (m.type == 5 || m.type == 6) {
            m.blow_dis = 0.0f;
        }
    }

    m.speed_rate = 0.5f;
    switch (hit_type) {
    case 1:
        m.hp = static_cast<int>(m.hp - (hit_point * magic_res_for_type(m.type)));
        break;
    default:
        m.hp -= hit_point;
        break;
    }

    m.be_hit_cur_ms = kBeHitTime;
    if (!is_rep()) {
        m.blood_show_ms = 1000;
    }
    return true;
}

void MainGameScene::monster_apply_magic_effect(size_t idx, int magic_type, int power, int spe_time) {
    if (idx >= monsters_.size()) {
        return;
    }

    auto& m = monsters_[idx];
    if (m.type >= 4) {
        return;
    }

    switch (magic_type) {
    case 1:
    case 2:
    case 3:
        m.burn_time_ms = spe_time * 1000;
        m.burn_hurt = power / 10;
        break;
    case 4:
    case 5:
    case 6:
        m.freeze_time_ms = spe_time * 1000;
        break;
    case 7:
    case 8:
    case 9:
        m.shock_time_ms = spe_time * 1000;
        break;
    default:
        break;
    }
}

// -----------------------------------------------------------------------
// spawn_monster
// -----------------------------------------------------------------------
void MainGameScene::spawn_monster(bool boss, int forced_type, int wait_ms, int mons_y_units) {
    MonsterEntity m;
    m.uid = next_monster_uid_++;
    m.boss = boss;

    if (boss) {
        m.type = (forced_type >= 0) ? forced_type
                                    : ((Param::stage / 10) % 2 == 0 ? 4 : 5);
    } else {
        if (forced_type >= 0) {
            m.type = forced_type;
        } else {
            const int roll = static_cast<int>(form_rng_() % 100);
            if (Param::stage >= 30 && roll < 16) {
                m.type = 6;
            } else {
                m.type = static_cast<int>(form_rng_() % 4);
            }
        }
    }

    const MonsterConfig& cfg = monster_config_for(m.type);
    m.w = monster_frame_size(cfg.size_frame_key, this, true);
    m.h = monster_frame_size(cfg.size_frame_key, this, false);
    m.speed = compute_monster_speed(cfg.run_speed_units, this);
    m.max_hp = compute_monster_hp(cfg.base_hp, m.type);
    m.atk = cfg.atk;
    m.atk_cd_ms = cfg.atk_cd_ms;
    m.run_frame_num = cfg.run_frame_num;
    m.atk_frame_num = cfg.atk_frame_num;
    m.atk2_frame_num = cfg.atk2_frame_num;
    m.die_frame_num = cfg.die_frame_num;
    m.jump_frame_num = cfg.jump_frame_num;
    m.atk_judge_frame = cfg.atk_judge_frame;
    m.atk_judge_frame2 = cfg.atk_judge_frame2;
    m.remote_missile_frames = cfg.remote_missile_frames;
    m.missile_blast_frames = cfg.missile_blast_frames;
    m.is_remote_atk = cfg.is_remote_atk;

    m.hp = m.max_hp;
    m.x  = screen_width;
    m.y  = (mons_y_units == -10)
        ? get_y(static_cast<float>(form_rng_() % 330 + 90))
        : get_y(static_cast<float>(mons_y_units));

    if (m.type == 5) {
        m.x += get_x(100.0f);
    }

    // FIX 1: Set monster_atk_x so monsters know when to stop and attack.
    // They halt just to the right of the wall with a small per-type offset.
    m.monster_atk_x = attack_x_for_type(m.type, this);

    // FIX 2: Set is_remote_atk for ranged types so missile helpers activate.
    m.is_remote_atk = (m.type == 3 || m.type == 6);

    // FIX 3: Start as kRunning immediately — wait_time_ms stays 0.
    // The kWaiting path in update_monsters is kept for external callers
    // that want a delayed spawn (set wait_time_ms > 0 after calling this).
    m.status       = MonsterStatus::kWaiting;
    m.wait_time_ms = wait_ms;

    m.speed_rate       = 1.0f;
    m.shock_speed_rate = 1.0f;
    m.blow_dis         = 0.0f;
    m.shock_time_ms    = 0;
    m.freeze_time_ms   = 0;
    m.burn_time_ms     = 0;
    m.burn_hurt        = 0;
    m.be_hit_cur_ms    = 0;
    m.blood_show_ms    = 0;
    m.img_alpha        = 1.0f;
    m.die_alpha        = 1.0f;
    m.img_time_ms      = 0;
    m.img_id           = 0;
    m.atk_judge        = false;
    m.fire_flag        = false;
    m.summon_flag      = true;
    m.summon_time_ms   = 0;
    m.run_time_ms      = 0;
    m.burn_judge       = false;
    m.spe_id           = 0;
    m.jump_time_ms     = 0;
    m.target_dx        = 1.0f;
    m.target_dy        = 0.0f;
    m.sto_lag_ms       = 0;
    m.sto_atk_flag     = false;
    m.sto_speed_x      = 0.0f;
    m.sto_speed_y      = 0.0f;
    m.sto_angle        = 0.0f;
    m.missile_x        = 0.0f;
    m.missile_y        = 0.0f;
    m.remote_img_time  = 0;
    m.remote_img_id    = 0;
    m.disturbDis       = 0.0f;
    m.is_in_river      = false;
    m.river_slow_rate  = 1.0f;
    m.delta_drop_y     = get_y(6.0f);
    m.needs_projectile = false;
    m.pending_summon_count = 0;
    m.atk_accum_ms     = 0;
    m.survive_time_ms  = 0;
    m.attacking        = false;

    monsters_.push_back(m);
}

// -----------------------------------------------------------------------
// spawn_enemy_projectile
// -----------------------------------------------------------------------
void MainGameScene::spawn_enemy_projectile(const MonsterEntity& mon) {
    if (mon.type != 3 && mon.type != 5 && mon.type != 6) return;

    EnemyProjectile p;
    if (mon.type == 5)      p.kind = kProjectileKindBossFire;
    else if (mon.type == 6) p.kind = kProjectileKindStone;
    else                    p.kind = kProjectileKindDevilFire;

    p.x           = mon.x - (mon.w * 0.35f);
    p.y           = mon.y - (mon.h * 0.10f);
    p.damage      = std::max(1, mon.atk);
    p.blasting    = false;
    p.lifetime_ms = 0.0f;

    const float target_x = wall_x_ - get_x(4.0f);
    const float target_y = mon.y + get_y(static_cast<float>(
        static_cast<int>(form_rng_() % 30) - 15));
    float dx = target_x - p.x;
    float dy = target_y - p.y;
    const float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.001f) { dx /= len; dy /= len; }
    else              { dx = -1.0f; dy = 0.0f; }

    float speed = get_x(350.0f);
    if (p.kind == kProjectileKindBossFire)  speed = get_x(290.0f);
    else if (p.kind == kProjectileKindStone) speed = get_x(245.0f);

    p.vx = dx * speed;
    p.vy = dy * speed;
    enemy_projectiles_.push_back(p);
}

// -----------------------------------------------------------------------
// update_enemy_projectiles
// -----------------------------------------------------------------------
void MainGameScene::update_enemy_projectiles(float dt_ms) {
    const float dt = dt_ms / 1000.0f;
    for (size_t i = 0; i < enemy_projectiles_.size();) {
        EnemyProjectile& p = enemy_projectiles_[i];
        p.lifetime_ms += dt_ms;

        if (!p.blasting) {
            p.x += p.vx * dt;
            p.y += p.vy * dt;

            if (p.x <= wall_x_ + get_x(8.0f)) {
                p.blasting    = true;
                p.lifetime_ms = 0.0f;
                wall_hp_     -= p.damage;
                Param::life_percent = (wall_hp_ * 100) / std::max(1, wall_max_hp_);
                if (wall_hp_ <= 0) {
                    wall_hp_      = 0;
                    Param::life_percent = 0;
                    is_game_over_ = true;
                }
            }
        }

        if (p.blasting) {
            if (p.lifetime_ms > 520.0f) {
                enemy_projectiles_.erase(enemy_projectiles_.begin() +
                    static_cast<std::ptrdiff_t>(i));
                continue;
            }
        } else if (p.x < -get_x(60.0f) ||
                   p.y < -get_y(60.0f) ||
                   p.y > screen_height + get_y(60.0f)) {
            enemy_projectiles_.erase(enemy_projectiles_.begin() +
                static_cast<std::ptrdiff_t>(i));
            continue;
        }
        ++i;
    }
}

// -----------------------------------------------------------------------
// update_status_effects
// -----------------------------------------------------------------------
static void update_status_effects(MainGameScene::MonsterEntity& m, float dt_ms,
                                  MainGameScene* scene) {
    if (m.speed_rate < 1.0f) {
        m.speed_rate += dt_ms / 500.0f;
        if (m.speed_rate > 1.0f) m.speed_rate = 1.0f;
    }
    if (m.shock_time_ms > 0) {
        m.shock_time_ms    -= static_cast<int>(dt_ms);
        m.shock_speed_rate  = 0.5f;
        m.spe_id = (static_cast<int>(AbstractGame::game_time_ms() / 80)) % 4;
    } else {
        m.shock_speed_rate = 1.0f;
    }
    if (m.freeze_time_ms > 0) {
        m.freeze_time_ms -= static_cast<int>(dt_ms);
    }
    if (m.burn_time_ms > 0 || m.is_in_river) {
        m.spe_id = (static_cast<int>(AbstractGame::game_time_ms() / 80)) % 6;
        if (m.burn_time_ms > 0) {
            m.burn_time_ms -= static_cast<int>(dt_ms);
            if (m.spe_id == 0) m.burn_judge = true;
            if (m.spe_id == 5 && m.burn_judge &&
                m.status != MonsterStatus::kDying) {
                m.hp -= m.burn_hurt;
                m.burn_judge = false;
            }
        }
    }
    if (m.be_hit_cur_ms > 0) {
        m.be_hit_cur_ms -= static_cast<int>(dt_ms);
        if (m.be_hit_cur_ms < 0) m.be_hit_cur_ms = 0;
    }
    if (m.blood_show_ms > 0) {
        m.blood_show_ms -= static_cast<int>(dt_ms);
    }
}

// -----------------------------------------------------------------------
// act_normal — types 0-3
// -----------------------------------------------------------------------
static void act_normal(MainGameScene::MonsterEntity& m, float dt_ms,
                       float wall_x, bool is_ranged, bool& wall_hit,
                       int& wall_dmg, MainGameScene* scene) {
    const int idt_ms = static_cast<int>(dt_ms);

    switch (m.status) {
    case MonsterStatus::kDying:
        m.img_time_ms += idt_ms;
        m.img_id = (m.img_time_ms / kFrameMs) + m.atk_frame_num + m.run_frame_num;
        if (m.img_id >= m.die_frame_num + m.atk_frame_num + m.run_frame_num) {
            m.img_id = m.die_frame_num + m.atk_frame_num + m.run_frame_num - 1;
        }
        m.die_alpha = (kDieShowTime - m.img_time_ms) / static_cast<float>(kDieShowTime);
        if (m.img_time_ms > kDieShowTime) {
            m.status = MonsterStatus::kRemove;
        }
        break;

    case MonsterStatus::kRunning: {
        if (m.blow_dis > 0.0f) {
            float temp = m.blow_dis * 3.0f;
            if (temp < kBlowMinSpeed) temp = kBlowMinSpeed;
            m.x        += (scene->get_x(temp) * dt_ms) / 500.0f;
            m.blow_dis -= (scene->get_x(temp) * dt_ms) / 500.0f;
            if (m.blow_dis <= 0.0f) {
                m.blow_dis   = 0.0f;
                m.speed_rate = 0.5f;
            }
        } else if (m.freeze_time_ms <= 0) {
            m.x -= ((m.speed * m.speed_rate * dt_ms * m.shock_speed_rate * m.river_slow_rate) / 1000.0f);
        }
        m.img_time_ms += static_cast<int>(dt_ms * m.speed_rate * m.shock_speed_rate);
        m.img_id = (m.img_time_ms / kFrameMs) % m.run_frame_num;
        if (m.x <= m.monster_atk_x) {
            m.x           = m.monster_atk_x;
            m.img_time_ms = 0;
            m.status      = MonsterStatus::kAttacking;
        }
        m.atk_accum_ms -= static_cast<int>(dt_ms * m.speed_rate * m.shock_speed_rate);
        break;
    }

    case MonsterStatus::kAttacking: {
        if (m.blow_dis > 0.0f) {
            float temp = m.blow_dis * 3.0f;
            if (temp < kBlowMinSpeed) temp = kBlowMinSpeed;
            m.x        += (scene->get_x(temp) * dt_ms) / 500.0f;
            m.blow_dis -= (scene->get_x(temp) * dt_ms) / 500.0f;
            if (m.blow_dis <= 0.0f) {
                m.blow_dis   = 0.0f;
                m.speed_rate = 0.5f;
            }
        }
        if (m.atk_accum_ms <= 0 && m.freeze_time_ms <= 0) {
            m.img_time_ms += static_cast<int>(dt_ms * m.speed_rate * m.shock_speed_rate);
            const int prev_id = m.img_id;
            m.img_id = (m.img_time_ms / kFrameMs) + m.run_frame_num;
            if (m.img_id != prev_id) m.atk_judge = false;
            if (m.img_id >= m.atk_frame_num + m.run_frame_num - 1) {
                m.img_time_ms  = 0;
                m.atk_accum_ms = m.atk_cd_ms;
            }
            if (m.img_id == (m.atk_judge_frame + m.run_frame_num - 1) && !m.atk_judge) {
                if (is_ranged) {
                    m.needs_projectile = true;
                } else {
                    wall_hit  = true;
                    wall_dmg += m.atk;
                }
                m.atk_judge = true;
            }
        } else {
            m.atk_accum_ms -= static_cast<int>(dt_ms * m.speed_rate * m.shock_speed_rate);
            m.img_time_ms  += static_cast<int>(dt_ms * m.speed_rate * m.shock_speed_rate);
            m.img_id        = 0;
            if (m.atk_accum_ms <= 0) m.img_time_ms = 0;
        }
        if (m.x > m.monster_atk_x + scene->get_x(50.0f)) {
            m.status = MonsterStatus::kRunning;
        }
        break;
    }

    default:
        break;
    }
}

// -----------------------------------------------------------------------
// act_boss1 — type 4
// -----------------------------------------------------------------------
static void act_boss1(MainGameScene::MonsterEntity& m, float dt_ms, bool& wall_hit,
                      int& wall_dmg, MainGameScene* scene) {
    const int idt_ms = static_cast<int>(dt_ms);

    switch (m.status) {
    case MonsterStatus::kDying:
        m.img_time_ms += idt_ms;
        m.img_id = (m.img_time_ms / kFrameMs) + m.atk_frame_num + m.run_frame_num;
        if (m.img_id >= m.die_frame_num + m.atk_frame_num + m.run_frame_num)
            m.img_id = m.die_frame_num + m.atk_frame_num + m.run_frame_num - 1;
        m.die_alpha = (kDieShowTime - m.img_time_ms) / static_cast<float>(kDieShowTime);
        if (m.img_time_ms > kDieShowTime) m.status = MonsterStatus::kRemove;
        break;

    case MonsterStatus::kRunning:
        if (m.jump_time_ms > 0) {
            m.x += (m.target_dx * dt_ms) / 500.0f;
            m.y += (m.target_dy * dt_ms) / 500.0f;
            m.img_time_ms += idt_ms;
            m.img_id = ((m.img_time_ms / kBoss1JumpFrameMs) % m.jump_frame_num)
                       + m.run_frame_num + m.atk_frame_num + m.die_frame_num;
            m.jump_time_ms -= idt_ms;
            if (m.jump_time_ms <= 0) m.img_time_ms = 0;
        } else {
            if (m.blow_dis > 0.0f) {
                float temp = m.blow_dis * 3.0f;
                if (temp < kBlowMinSpeed) temp = kBlowMinSpeed;
                m.x        += (scene->get_x(temp) * dt_ms) / 500.0f;
                m.blow_dis -= (2.0f * scene->get_x(temp) * dt_ms) / 500.0f;
                if (m.blow_dis <= 0.0f) {
                    m.blow_dis   = 0.0f;
                    m.speed_rate = 0.5f;
                }
            } else {
                m.x -= ((m.speed * m.speed_rate * dt_ms * m.shock_speed_rate) / 1000.0f);
            }
            m.img_time_ms += static_cast<int>(dt_ms * m.speed_rate * m.shock_speed_rate);
            m.img_id = (m.img_time_ms / kFrameMs) % m.run_frame_num;
            if (m.x <= m.monster_atk_x) {
                m.x           = m.monster_atk_x;
                m.img_time_ms = 0;
                m.status      = MonsterStatus::kAttacking;
            }
        }
        break;

    case MonsterStatus::kAttacking: {
        if (m.blow_dis > 0.0f) {
            float temp = m.blow_dis * 3.0f;
            if (temp < kBlowMinSpeed) temp = kBlowMinSpeed;
            m.x        += (scene->get_x(temp) * dt_ms) / 500.0f;
            m.blow_dis -= (scene->get_x(temp) * dt_ms) / 500.0f;
            if (m.blow_dis <= 0.0f) {
                m.blow_dis   = 0.0f;
                m.speed_rate = 0.5f;
            }
        }
        m.img_time_ms += static_cast<int>(dt_ms * m.speed_rate * m.shock_speed_rate);
        const int prev_id = m.img_id;
        m.img_id = ((m.img_time_ms / kFrameMs) % m.atk_frame_num) + m.run_frame_num;
        if (m.img_id != prev_id) m.atk_judge = false;

        if ((m.img_id == (m.atk_judge_frame  + m.run_frame_num - 1) ||
             m.img_id == (m.atk_judge_frame2 + m.run_frame_num - 1)) && !m.atk_judge) {
            wall_hit    = true;
            wall_dmg   += m.atk;
            m.atk_judge = true;
        }
        if (m.x > m.monster_atk_x + scene->get_x(50.0f)) {
            m.status = MonsterStatus::kRunning;
        }
        break;
    }

    default:
        break;
    }
}

// -----------------------------------------------------------------------
// act_boss2 — type 5
// -----------------------------------------------------------------------
static void act_boss2(MainGameScene::MonsterEntity& m, float dt_ms, bool& wall_hit,
                      int& wall_dmg, MainGameScene* scene, std::mt19937& rng) {
    const int idt_ms = static_cast<int>(dt_ms);

    switch (m.status) {
    case MonsterStatus::kDying:
        m.img_time_ms += idt_ms;
        m.img_id = (m.img_time_ms / kFrameMs) + m.atk_frame_num + m.run_frame_num;
        if (m.img_id >= m.die_frame_num + m.atk_frame_num + m.run_frame_num)
            m.img_id = m.die_frame_num + m.atk_frame_num + m.run_frame_num - 1;
        m.die_alpha = (kDieShowTime - m.img_time_ms) / static_cast<float>(kDieShowTime);
        if (m.img_time_ms > kDieShowTime) m.status = MonsterStatus::kRemove;
        break;

    case MonsterStatus::kRunning:
        if (m.disturbDis > 0.0f) {
            m.img_id      = m.run_frame_num + 2 + m.atk_frame_num;
            m.x          += dt_ms / 20.0f;
            m.disturbDis -= dt_ms / 20.0f;
            break;
        }
        if (m.run_time_ms < 4000) {
            const int prev_mod = m.run_time_ms % 1500;
            m.run_time_ms += idt_ms;
            if (m.run_time_ms % 1500 < prev_mod) {
                m.target_dx = scene->get_x(static_cast<float>(rng() % 100 + 500)) - m.x;
                m.target_dy = scene->get_y(static_cast<float>(rng() % 300 + 100)) - m.y;
                if (m.atk_judge) {
                    wall_hit    = true;
                    wall_dmg   += (m.atk * 3) / 2;
                    m.atk_judge = false;
                } else if (rng() % 100 < 25) {
                    m.target_dx = scene->get_x(150.0f) - m.x;
                    m.atk_judge = true;
                }
            }
            m.x += (m.target_dx * dt_ms) / 1500.0f;
            m.y += (m.target_dy * dt_ms) / 1500.0f;
            m.img_time_ms += idt_ms;
            m.img_id = (m.img_time_ms / kFrameMs) % m.run_frame_num;

            if (m.run_time_ms >= 4000) {
                m.atk_judge = false;
                if (rng() % 100 < 40) {
                    m.img_time_ms = 0;
                    m.status      = MonsterStatus::kAttacking;
                    m.run_time_ms = 0;
                } else {
                    m.summon_time_ms = 4500;
                    m.img_time_ms    = 0;
                }
            }
        }
        if (m.summon_time_ms > 0) {
            m.summon_time_ms -= idt_ms;
            if (m.summon_time_ms > 3000) {
                m.img_time_ms += idt_ms;
                m.img_id = (m.img_time_ms / kFrameMs) + m.atk_frame_num + m.run_frame_num + m.die_frame_num;
                if (m.img_time_ms / kFrameMs == 8 && m.summon_flag) {
                    m.pending_summon_count = (Param::stage / 15) + 3;
                    m.summon_flag          = false;
                }
            } else {
                m.img_time_ms += idt_ms;
                m.img_id = (m.img_time_ms / kFrameMs) % m.run_frame_num;
                if (m.x < scene->get_x(720.0f)) {
                    m.x += (m.speed * dt_ms) / 1000.0f;
                } else {
                    m.x = scene->get_x(720.0f);
                }
                if (m.summon_time_ms <= 0) {
                    m.run_time_ms = 0;
                    m.target_dx   = scene->get_x(static_cast<float>(rng() % 100 + 500)) - m.x;
                    m.target_dy   = scene->get_y(static_cast<float>(rng() % 300 + 100)) - m.y;
                    m.summon_flag = true;
                }
            }
        }
        break;

    case MonsterStatus::kAttacking:
        if (m.x < scene->get_x(720.0f)) {
            m.x += (m.speed * 1.5f * dt_ms) / 1000.0f;
            if (m.x > scene->get_x(720.0f)) m.x = scene->get_x(720.0f);
            m.img_time_ms += idt_ms;
            m.img_id = (m.img_time_ms / kFrameMs) % m.run_frame_num;
        } else if (m.run_time_ms < 1500) {
            m.img_time_ms += idt_ms;
            m.img_id       = (m.img_time_ms / kFrameMs) % m.run_frame_num;
            m.img_alpha    = (2000 - m.run_time_ms) / 2000.0f;
            m.run_time_ms += idt_ms;
            if (m.run_time_ms >= 1500) {
                m.run_time_ms = 1500;
                m.img_time_ms = 0;
            }
        } else if (!m.fire_flag) {
            m.img_time_ms += idt_ms;
            m.img_id = (m.img_time_ms / kFrameMs) + m.run_frame_num;
            if (m.img_time_ms / kFrameMs == 7) {
                m.fire_flag = true;
                m.img_alpha = 1.0f;
            }
            if (m.img_time_ms / kFrameMs == 9) {
                m.status      = MonsterStatus::kRunning;
                m.run_time_ms = 0;
                m.img_time_ms = 0;
                m.target_dx   = scene->get_x(static_cast<float>(rng() % 100 + 500)) - m.x;
                m.target_dy   = scene->get_y(static_cast<float>(rng() % 300 + 100)) - m.y;
            }
        } else {
            const int prev_mod = m.run_time_ms % 401;
            m.run_time_ms += idt_ms;
            if (m.run_time_ms % 401 < prev_mod) {
                wall_hit  = true;
                wall_dmg += m.atk;
            }
            if (m.run_time_ms > 3800) {
                m.fire_flag   = false;
                m.img_time_ms = 1000;
            }
        }
        break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------
// act_stone_machine — type 6
// -----------------------------------------------------------------------
static void act_stone_machine(MainGameScene::MonsterEntity& m, float dt_ms,
                              bool& wall_hit, int& wall_dmg,
                              MainGameScene* scene) {
    const int idt_ms = static_cast<int>(dt_ms);

    switch (m.status) {
    case MonsterStatus::kRunning:
        m.x -= ((m.speed * m.speed_rate * dt_ms) / 1000.0f);
        m.img_time_ms += static_cast<int>(dt_ms * m.speed_rate);
        m.img_id = (m.img_time_ms / kFrameMs) % m.run_frame_num;
        if (m.x <= m.monster_atk_x) {
            m.x            = m.monster_atk_x;
            m.img_time_ms  = 0;
            m.sto_lag_ms   = 1000;
            m.status       = MonsterStatus::kAttacking;
            m.sto_atk_flag = false;
        }
        break;

    case MonsterStatus::kAttacking:
        if (m.sto_lag_ms > 0) {
            m.sto_lag_ms -= idt_ms;
            break;
        }
        m.img_time_ms += static_cast<int>(dt_ms * m.speed_rate * m.shock_speed_rate);
        if (!m.sto_atk_flag) {
            m.img_id = ((m.img_time_ms / kStoneFrameMs) % m.atk2_frame_num)
                       + m.run_frame_num + m.atk_frame_num + m.die_frame_num;
            if (m.img_time_ms >= m.atk2_frame_num * kStoneFrameMs) {
                m.img_id       = m.run_frame_num + m.atk_frame_num - 1;
                m.img_time_ms  = 0;
                m.sto_lag_ms   = 2000;
                m.sto_atk_flag = true;
            }
        } else {
            if (m.atk_accum_ms <= 0) {
                const int prev_id = m.img_id;
                m.img_id = ((m.img_time_ms / kFrameMs) % m.atk_frame_num) + m.run_frame_num;
                if (m.img_id != prev_id) m.atk_judge = false;
                if (m.img_id >= m.atk_frame_num + m.run_frame_num - 1) {
                    m.img_time_ms  = 0;
                    m.atk_accum_ms = m.atk_cd_ms;
                    m.img_id       = m.atk_frame_num + m.run_frame_num - 1;
                }
                if (m.img_id == (m.atk_judge_frame + m.run_frame_num - 1) && !m.atk_judge) {
                    m.missile_x       = m.x - (m.w / 6.0f);
                    m.missile_y       = m.y + (m.h / 6.0f);
                    m.remote_img_id   = 0;
                    m.remote_img_time = 0;
                    m.sto_speed_x     = scene->get_x(500.0f) / 1000.0f;
                    m.sto_speed_y     = m.sto_speed_x / 2.0f;
                    m.sto_angle       = std::atan2(m.sto_speed_y, m.sto_speed_x);
                    m.atk_judge       = true;
                }
            } else {
                m.atk_accum_ms -= static_cast<int>(dt_ms * m.speed_rate * m.shock_speed_rate);
                m.img_time_ms   = 0;
                m.img_id        = m.atk_frame_num + m.run_frame_num - 1;
            }
        }
        break;

    case MonsterStatus::kDying:
        m.img_time_ms += idt_ms;
        m.img_id = (m.img_time_ms / kStoneFrameMs) + m.atk_frame_num + m.run_frame_num;
        if (m.img_id >= m.die_frame_num + m.atk_frame_num + m.run_frame_num)
            m.img_id = m.die_frame_num + m.atk_frame_num + m.run_frame_num - 1;
        m.die_alpha = (kDieShowTime - m.img_time_ms) / static_cast<float>(kDieShowTime);
        if (m.img_time_ms > kDieShowTime) m.status = MonsterStatus::kRemove;
        break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------
// update_stone_missile
// -----------------------------------------------------------------------
static void update_stone_missile(MainGameScene::MonsterEntity& m, float dt_ms,
                                 float wall_x, bool& wall_hit,
                                 int& wall_dmg, MainGameScene* scene) {
    if (!m.is_remote_atk || m.type != 6) return;

    const float hit_x = scene->get_x(100.0f);
    if (m.missile_x > hit_x) {
        m.missile_x      -= dt_ms * m.sto_speed_x;
        m.missile_y      += dt_ms * m.sto_speed_y;
        m.sto_speed_y    -= (m.sto_speed_x * dt_ms) / 750.0f;
        m.sto_angle       = std::atan2(m.sto_speed_y, m.sto_speed_x);
        m.remote_img_time += static_cast<int>(dt_ms);
        m.remote_img_id   = (m.remote_img_time / 80) % m.remote_missile_frames;
        if (m.missile_x <= hit_x) {
            wall_hit          = true;
            wall_dmg         += m.atk;
            m.missile_x       = hit_x;
            m.remote_img_time = 0;
        }
    }
    if (m.missile_x == hit_x) {
        m.remote_img_time += static_cast<int>(dt_ms);
        m.remote_img_id    = (m.remote_img_time / 80) + m.remote_missile_frames;
        if (m.remote_img_id == m.remote_missile_frames + m.missile_blast_frames) {
            m.missile_x = 0.0f;
        }
    }
}

// -----------------------------------------------------------------------
// update_devil_missile
// -----------------------------------------------------------------------
static void update_devil_missile(MainGameScene::MonsterEntity& m, float dt_ms,
                                 bool& wall_hit, int& wall_dmg,
                                 MainGameScene* scene) {
    if (!m.is_remote_atk || m.type != 3) return;

    const float hit_x = scene->get_x(150.0f);
    if (m.missile_x > hit_x) {
        m.missile_x      -= (dt_ms * scene->get_x(500.0f)) / 1000.0f;
        m.remote_img_time += static_cast<int>(dt_ms);
        m.remote_img_id   = (m.remote_img_time / 80) % m.remote_missile_frames;
        if (m.missile_x <= hit_x) {
            wall_hit          = true;
            wall_dmg         += m.atk;
            m.missile_x       = hit_x;
            m.remote_img_time = 0;
        }
    }
    if (m.missile_x == hit_x) {
        m.remote_img_time += static_cast<int>(dt_ms);
        m.remote_img_id    = (m.remote_img_time / 80) + m.remote_missile_frames;
        if (m.remote_img_id == m.remote_missile_frames + m.missile_blast_frames) {
            m.missile_x = 0.0f;
        }
    }
}

// -----------------------------------------------------------------------
// update_monsters — main update loop
// -----------------------------------------------------------------------
void MainGameScene::update_monsters(float dt_ms) {
    for (size_t i = 0; i < monsters_.size();) {
        auto& m = monsters_[i];

        // ---- Wait phase ----
        if (m.status == MonsterStatus::kWaiting) {
            if (m.wait_time_ms > 0) {
                m.wait_time_ms -= static_cast<int>(dt_ms);
            }
            if (m.wait_time_ms <= 0) {
                m.status = MonsterStatus::kRunning;
            } else {
                ++i;
                continue;
            }
        }

        m.survive_time_ms += static_cast<int>(dt_ms);
        m.attacking = (m.status == MonsterStatus::kAttacking);

        update_status_effects(m, dt_ms, this);

        bool wall_hit = false;
        int  wall_dmg = 0;

        if (m.type == 6) update_stone_missile(m, dt_ms, wall_x_, wall_hit, wall_dmg, this);
        if (m.type == 3) update_devil_missile(m, dt_ms, wall_hit, wall_dmg, this);

        m.needs_projectile = false;
        if (m.type < 4) {
            bool ranged = (m.type == 3);
            act_normal(m, dt_ms, wall_x_, ranged, wall_hit, wall_dmg, this);
        } else if (m.type == 4) {
            act_boss1(m, dt_ms, wall_hit, wall_dmg, this);
        } else if (m.type == 5) {
            act_boss2(m, dt_ms, wall_hit, wall_dmg, this, form_rng_);
            if (m.pending_summon_count > 0) {
                for (int s = 0; s < m.pending_summon_count; ++s) {
                    spawn_monster(false, static_cast<int>(form_rng_() % 4));
                }
                m.pending_summon_count = 0;
            }
        } else if (m.type == 6) {
            act_stone_machine(m, dt_ms, wall_hit, wall_dmg, this);
        }

        if (m.needs_projectile) {
            spawn_enemy_projectile(m);
        }

        m.attacking = (m.status == MonsterStatus::kAttacking);

        if (wall_hit && wall_dmg > 0) {
            wall_hp_ -= wall_dmg;
            Param::life_percent = (wall_hp_ * 100) / std::max(1, wall_max_hp_);
            if (wall_hp_ <= 0) {
                wall_hp_      = 0;
                Param::life_percent = 0;
                is_game_over_ = true;
                break;
            }
        }

        if (m.status == MonsterStatus::kRemove) {
            kill_monster(i);
            continue;
        }

        if (m.hp <= 0 && m.status != MonsterStatus::kDying &&
            m.status != MonsterStatus::kRemove) {
            m.hp          = 0;
            m.img_time_ms = 0;
            m.status      = MonsterStatus::kDying;
            m.fire_flag   = false;
            Param::kills       += 1;
            Param::total_kills += 1;
            AchvMng::check_achv_in_game(2);
            Param::gold += static_cast<int>(form_rng_() % 2) + 5 + Param::extra_gold;
            if (m.type == 6) {
                Param::gold += 100;
            }
            if (m.boss) boss_killed_ = true;
            effects_.push_back({m.x, m.y, get_x(16.0f), 260.0f, 1.0f, 0.75f, 0.2f});
        }

        ++i;
    }
}

} // namespace defender
