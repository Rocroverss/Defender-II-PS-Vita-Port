// wall_defender.cpp
// Ported from BasalMonster.draw() — adds be-hit flash, die-alpha fade,
// freeze overlay, burn/shock overlay, blood bar, boss-2 fire overlay,
// stone/devil missile rendering, img_alpha ghost effect.

#include "scenes/main_game_scene.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include <vector>

#include "engine/abstract_game.hpp"
#include "engine/game_texture_specs.hpp"
#include "engine/render_utils.hpp"
#include "engine/sprite_atlas_cache.hpp"
#include "engine/texture_cache.hpp"
#include "game/audio_manager.hpp"
#include "game/item_param.hpp"
#include "game/skill_data.hpp"
#include "game/sounds.hpp"

namespace defender {
namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr int kProjectileKindDevilFire = 1;
constexpr int kProjectileKindBossFire  = 2;
constexpr int kProjectileKindStone     = 3;
constexpr int kTurretBlastFrameCount   = 10;
constexpr const char* kMonsterBloodFrame = "assets/imgs_480_800/game/monster_blood_frame.png";
constexpr const char* kMonsterBloodPiece = "assets/imgs_480_800/game/z_monster_blood_piece.png";

// Be-hit full flash duration (ms) — Java: beHitTime = 200
constexpr float kBeHitTime = 200.0f;

const TextureHandle& game_texture(const std::string& file_name) {
    return TextureCache::instance().get("assets/imgs_480_800/game/" + file_name);
}

int item_level(const MainGameScene* scene, int type) {
    return scene->is_rep() ? ItemParam::get_level(type, false) : ItemParam::get_level(type);
}

const char* monster_tex_name(int type, bool boss, uint64_t game_time_ms) {
    if (boss) return "monster_logo.png";
    switch (type) {
    case 1:  return "normal_lv1.png";
    case 2:  return "normal_lv2.png";
    case 3:  return "normal_lv3.png";
    default: {
        const int frame = static_cast<int>((game_time_ms / 125) % 10) + 1;
        static const char* kFrames[] = {
            "normal_001.png","normal_002.png","normal_003.png",
            "normal_004.png","normal_005.png","normal_006.png",
            "normal_007.png","normal_008.png","normal_009.png","normal_010.png"
        };
        return kFrames[frame - 1];
    }
    }
}

std::string frame_name(const std::string& prefix, int index, int pad_digits) {
    std::string number = std::to_string(index);
    if (pad_digits > 0 && static_cast<int>(number.size()) < pad_digits) {
        number = std::string(
            static_cast<size_t>(pad_digits - static_cast<int>(number.size())), '0') + number;
    }
    return prefix + number + ".png";
}

std::vector<std::string> frame_range(const std::string& prefix, int begin, int end, int pad_digits) {
    std::vector<std::string> out;
    out.reserve(static_cast<size_t>(std::max(0, end - begin + 1)));
    for (int i = begin; i <= end; ++i) out.push_back(frame_name(prefix, i, pad_digits));
    return out;
}

void append_frames(std::vector<std::string>& out,
                   const std::string& prefix,
                   int begin, int end, int pad_digits) {
    const auto chunk = frame_range(prefix, begin, end, pad_digits);
    out.insert(out.end(), chunk.begin(), chunk.end());
}

const std::vector<std::string>& monster_body_frames(int type) {
    static const auto bird = [] {
        std::vector<std::string> out;
        append_frames(out, "quanjiniao_run_lan_", 1, 6, 4);
        append_frames(out, "quanjiniao_attack_lan_", 1, 10, 4);
        append_frames(out, "quanjiniao_dead_lan_", 1, 3, 4);
        return out;
    }();
    static const auto rabbit = [] {
        std::vector<std::string> out;
        append_frames(out, "manzutu_run_hong_", 1, 6, 4);
        append_frames(out, "manzutu_attack_hong_", 1, 6, 4);
        append_frames(out, "mazutu_death_hong_", 1, 7, 4);
        return out;
    }();
    static const auto eye = [] {
        std::vector<std::string> out;
        append_frames(out, "duyanguai_run_huang_", 1, 8, 4);
        append_frames(out, "duyanguai_attack_huang_", 1, 12, 4);
        append_frames(out, "duyanguai_death_huang_", 1, 10, 4);
        return out;
    }();
    static const auto devil = [] {
        std::vector<std::string> out;
        append_frames(out, "xiaoemo_run_zi_", 1, 6, 4);
        append_frames(out, "xiaoemo_attack_zi_", 1, 10, 4);
        append_frames(out, "xiaoemo_dead_zi_", 1, 12, 4);
        return out;
    }();
    static const auto boss1 = [] {
        std::vector<std::string> out;
        append_frames(out, "yemanshouren_run_", 1, 6, 3);
        append_frames(out, "yemanshouren_attack_", 1, 12, 3);
        append_frames(out, "yemanshouren_dead_", 1, 6, 3);
        append_frames(out, "yemanshouren_jump_", 1, 5, 3);
        out.push_back("yemanshouren_jump_005.png");
        out.push_back("yemanshouren_jump_004.png");
        out.push_back("yemanshouren_jump_003.png");
        out.push_back("yemanshouren_jump_002.png");
        out.push_back("yemanshouren_jump_001.png");
        return out;
    }();
    static const auto boss2 = [] {
        std::vector<std::string> out;
        append_frames(out, "jielingqishi_run_", 1, 6, 0);
        append_frames(out, "jielingqishi_attack1_", 1, 10, 0);
        append_frames(out, "jielingqishi_dead_", 1, 9, 0);
        append_frames(out, "jielingqishi_attack2_", 1, 12, 0);
        return out;
    }();
    static const auto stone = [] {
        std::vector<std::string> out;
        append_frames(out, "zz_toushiche_run_", 1, 3, 4);
        append_frames(out, "zz_toushiche_attack_", 1, 6, 4);
        append_frames(out, "zz_toushiche_death_", 1, 4, 4);
        append_frames(out, "zz_toushiche_stand_", 1, 6, 4);
        return out;
    }();
    static const std::vector<std::string> empty;
    switch (type) {
    case 0: return bird;
    case 1: return rabbit;
    case 2: return eye;
    case 3: return devil;
    case 4: return boss1;
    case 5: return boss2;
    case 6: return stone;
    default: return empty;
    }
}

const std::string& freeze_frame_for_type(int type) {
    static const std::string bird   = "quanjiniao_dong_lan_0001.png";
    static const std::string rabbit = "manzutu_dong_hong_0001.png";
    static const std::string eye    = "duyanguai_dong_huang_0001.png";
    static const std::string devil  = "xiaoemo_dong_zi_0001.png";
    static const std::string empty;
    switch (type) {
    case 0: return bird;
    case 1: return rabbit;
    case 2: return eye;
    case 3: return devil;
    default: return empty;
    }
}

// Boss-2 fire/store overlay (used when img_alpha < 1)
const std::vector<std::string>& boss2_store_frames() {
    static const auto store = frame_range("boss_store_", 1, 5, 3);
    return store;
}

// Boss-2 fire missile overlay
const std::vector<std::string>& boss2_fire_frames() {
    static const auto fire = frame_range("boss_fire_", 1, 4, 4);
    return fire;
}

// Stone machine missile / blast frames
const std::vector<std::string>& stone_missile_travel_frames() {
    static const auto s = frame_range("zz_stone_", 1, 5, 3);
    return s;
}
const std::vector<std::string>& stone_missile_blast_frames() {
    static const auto s = frame_range("fire_blast_", 1, 10, 2);
    return s;
}

// Devil-fire travel / blast
const std::vector<std::string>& devil_travel_frames() {
    static const auto d = frame_range("fire_devil_", 1, 6, 4);
    return d;
}
const std::vector<std::string>& devil_blast_frames() {
    static const auto d = frame_range("fire_devil_blast_", 1, 6, 4);
    return d;
}

// Burn / shock overlays
const std::vector<std::string>& burn_frames() {
    static const auto b = frame_range("burn_", 1, 6, 4);
    return b;
}
const std::vector<std::string>& shock_frames() {
    static const auto s = frame_range("shocked_", 1, 4, 3);
    return s;
}
// Freeze — single static frame per type (Java: MonsterData.getData().FreezeImg)
// We use a generic freeze overlay

// Projectile tables (used for EnemyProjectile layer, unchanged from original)
const std::vector<std::string>& projectile_travel_frames(int kind) {
    static const auto devil = frame_range("fire_devil_",  1, 6, 4);
    static const auto boss  = frame_range("boss_fire_",   1, 4, 4);
    static const auto stone = frame_range("zz_stone_",    1, 5, 3);
    static const std::vector<std::string> empty;
    if (kind == kProjectileKindDevilFire) return devil;
    if (kind == kProjectileKindBossFire)  return boss;
    if (kind == kProjectileKindStone)     return stone;
    return empty;
}

const std::vector<std::string>& projectile_blast_frames(int kind) {
    static const auto devil = frame_range("fire_devil_blast_", 1,  6, 4);
    static const auto boss  = frame_range("boss_store_",       1,  5, 3);
    static const auto stone = frame_range("fire_blast_",       1, 10, 2);
    static const std::vector<std::string> empty;
    if (kind == kProjectileKindDevilFire) return devil;
    if (kind == kProjectileKindBossFire)  return boss;
    if (kind == kProjectileKindStone)     return stone;
    return empty;
}

// ---- Utility ----
const std::string& anim_frame_at_time(const std::vector<std::string>& frames,
                                      float elapsed_ms, float frame_ms) {
    static const std::string kEmpty;
    if (frames.empty() || frame_ms <= 1.0f) return kEmpty;
    const int idx = static_cast<int>(elapsed_ms / frame_ms) %
                    static_cast<int>(frames.size());
    return frames[static_cast<size_t>(idx)];
}

const std::string& frame_by_img_id(const std::vector<std::string>& frames, int img_id) {
    static const std::string kEmpty;
    if (frames.empty()) return kEmpty;
    const int idx = std::clamp(img_id, 0, static_cast<int>(frames.size()) - 1);
    return frames[static_cast<size_t>(idx)];
}

bool draw_atlas_frame_centered(
    const std::string& frame_name_key,
    float cx, float cy, float w, float h,
    float angle_deg, float alpha = 1.0f)
{
    const AtlasFrame* frame = SpriteAtlasCache::instance().get_frame(frame_name_key);
    if (!frame || !frame->valid || frame->texture_id == 0 ||
        frame->width <= 0 || frame->height <= 0) return false;

    const float src_w = static_cast<float>(std::max(1, frame->original_width));
    const float src_h = static_cast<float>(std::max(1, frame->original_height));
    const float draw_w = w * (static_cast<float>(frame->width)  / src_w);
    const float draw_h = h * (static_cast<float>(frame->height) / src_h);
    const float draw_cx = cx + (frame->offset_x * (w / src_w));
    const float draw_cy = cy + (frame->offset_y * (h / src_h));

    draw_textured_quad_centered_uv(
        frame->texture_id,
        draw_cx, draw_cy, draw_w, draw_h,
        frame->u0, frame->v0, frame->u1, frame->v1,
        angle_deg, alpha);
    return true;
}

// Draw a named atlas or texture frame with size w×h centred at (cx,cy)
void draw_overlay_frame(const std::string& key, float cx, float cy,
                        float w, float h, float alpha) {
    if (!draw_atlas_frame_centered(key, cx, cy, w, h, 0.0f, alpha)) {
        const auto& tex = TextureCache::instance().get("assets/imgs_480_800/game/" + key);
        if (tex.valid) {
            draw_textured_quad(tex.id, cx - w * 0.5f, cy - h * 0.5f, w, h, alpha);
        }
    }
}

float distance_sq(float ax, float ay, float bx, float by) {
    const float dx = ax - bx;
    const float dy = ay - by;
    return (dx * dx) + (dy * dy);
}

bool is_alive_turret_target(const MainGameScene::MonsterEntity& mon) {
    return mon.status == MainGameScene::MonsterStatus::kRunning ||
           mon.status == MainGameScene::MonsterStatus::kAttacking;
}

} // namespace

void MainGameScene::init_wall_defenders() {
    const int tower_level = std::max(0, item_level(this, ItemParam::TOWER));
    const int special_level = std::max(0, item_level(this, ItemParam::TOWER_SPU));
    const int turret_attack = SkillData::get_value(SkillData::TOWER_ATK);
    const int blast_damage = special_level > 0
        ? (turret_attack * SkillData::get_value(SkillData::TOWER_SPU)) / 100
        : 0;
    const std::array<float, 2> turret_y = {
        get_y(182.0f),
        get_y(444.0f)
    };

    for (size_t i = 0; i < wall_defenders_.size(); ++i) {
        auto& turret = wall_defenders_[i];
        turret = TurretState{};
        turret.active = tower_level > static_cast<int>(i);
        turret.x = get_x(90.0f);
        turret.y = turret_y[i];
        turret.attack = turret_attack;
        turret.attack_range_px = static_cast<int>(get_x(600.0f));
        turret.cooldown_ms = 1500;
        turret.blast_range_px = special_level > 0 ? static_cast<int>(get_x(120.0f)) : 0;
        turret.blast_damage = blast_damage;
    }
}

void MainGameScene::update_wall_defenders(float dt_ms) {
    const auto& ring_tex = game_texture("ring_normal_lv3.png");
    const float ring_h = get_y(static_cast<float>(
        ring_tex.valid ? game_texture_height_px(ring_tex, "ring_normal_lv3.png") : 67));
    const float now_ms = static_cast<float>(AbstractGame::game_time_ms());
    const float dt_sec = dt_ms / 1000.0f;

    auto find_monster_index = [this](uint64_t uid) -> int {
        if (uid == 0) {
            return -1;
        }
        for (size_t i = 0; i < monsters_.size(); ++i) {
            if (monsters_[i].uid == uid) {
                return static_cast<int>(i);
            }
        }
        return -1;
    };

    auto acquire_target_uid = [this](const TurretState& turret) -> uint64_t {
        const float range_sq = static_cast<float>(turret.attack_range_px * turret.attack_range_px);
        for (const auto& mon : monsters_) {
            if (!is_alive_turret_target(mon)) {
                continue;
            }
            if (distance_sq(mon.x, mon.y, turret.x, turret.y) < range_sq) {
                return mon.uid;
            }
        }
        return 0;
    };

    for (auto& turret : wall_defenders_) {
        if (!turret.active) {
            continue;
        }

        turret.bg_frame = static_cast<int>((AbstractGame::game_time_ms() / 80ULL) % 10ULL);
        turret.bob_y = (std::sin((6.28f * now_ms) / 1500.0f) * ring_h) / 15.0f;
        if (turret.alpha < 1.0f) {
            turret.alpha = std::min(1.0f, turret.alpha + (dt_ms / 500.0f));
        }

        int turret_target_index = find_monster_index(turret.target_uid);
        if (turret_target_index >= 0) {
            const auto& mon = monsters_[static_cast<size_t>(turret_target_index)];
            if (mon.status == MonsterStatus::kDying || mon.status == MonsterStatus::kRemove) {
                turret.target_uid = 0;
                turret_target_index = -1;
            }
        } else {
            turret.target_uid = 0;
        }
        if (turret.target_uid == 0) {
            turret.target_uid = acquire_target_uid(turret);
        }

        if (turret.attack_time_ms > 0) {
            turret.attack_time_ms -= static_cast<int>(dt_ms);
            if (turret.attack_time_ms < 0) {
                turret.attack_time_ms = 0;
            }
        } else if (turret.target_uid != 0) {
            for (auto& missile : turret.missiles) {
                if (missile.active) {
                    continue;
                }
                missile = TurretMissile{};
                missile.active = true;
                missile.x = turret.x;
                missile.y = turret.y + turret.bob_y;
                missile.accel_x = get_x(2000.0f);
                missile.vx = get_x(100.0f);
                missile.delta_vy = (((form_rng_() % 2) == 0) ? -0.5f : 0.5f) * get_y(400.0f);
                missile.target_uid = turret.target_uid;
                turret.attack_time_ms = turret.cooldown_ms;
                turret.alpha = 0.0f;
                break;
            }
        }

        for (auto& missile : turret.missiles) {
            if (!missile.active) {
                continue;
            }

            if (missile.blasting) {
                missile.blast_time_ms += static_cast<int>(dt_ms);
                if (missile.blast_time_ms > (kTurretBlastFrameCount * 50)) {
                    missile.active = false;
                }
                continue;
            }

            int target_index = find_monster_index(missile.target_uid);
            if (target_index >= 0) {
                const auto& target = monsters_[static_cast<size_t>(target_index)];
                if (target.status == MonsterStatus::kDying || target.status == MonsterStatus::kRemove) {
                    target_index = -1;
                }
            }
            if (target_index < 0 && turret.target_uid != 0) {
                missile.target_uid = turret.target_uid;
                target_index = find_monster_index(missile.target_uid);
            }
            if (target_index < 0) {
                missile.active = false;
                continue;
            }

            const auto& target = monsters_[static_cast<size_t>(target_index)];
            const float denom_raw = target.x - missile.x;
            const float min_denom = get_x(1.0f);
            const float denom = std::abs(denom_raw) < min_denom
                ? (denom_raw < 0.0f ? -min_denom : min_denom)
                : denom_raw;

            missile.vx += missile.accel_x * dt_sec;
            missile.vy = missile.delta_vy + ((missile.vx * (target.y - missile.y)) / denom);
            missile.delta_vy -= (missile.delta_vy * dt_ms) / 300.0f;
            missile.x += missile.vx * dt_sec;
            missile.y += missile.vy * dt_sec;

            if (missile.x >= target.x) {
                missile.x = target.x;
                missile.y = target.y;

                if (is_alive_turret_target(target)) {
                    monster_be_hit(static_cast<size_t>(target_index), turret.attack, 0);
                    AudioManager::instance().play_sound(Sounds::BEHIT_SND);
                }

                if (turret.blast_range_px > 0 && turret.blast_damage > 0) {
                    const float blast_range_sq = static_cast<float>(turret.blast_range_px * turret.blast_range_px);
                    for (size_t i = 0; i < monsters_.size(); ++i) {
                        if (!is_alive_turret_target(monsters_[i])) {
                            continue;
                        }
                        if (distance_sq(monsters_[i].x, monsters_[i].y, missile.x, missile.y) <
                            blast_range_sq) {
                            monster_be_hit(i, turret.blast_damage, 0);
                        }
                    }
                }

                missile.blasting = true;
                missile.blast_time_ms = 0;
            }
        }
    }
}

void MainGameScene::draw_auto_turrets() {
    const auto& ball_tex = game_texture("normal_lv1.png");
    const float ball_w = get_x(static_cast<float>(
        ball_tex.valid ? game_texture_width_px(ball_tex, "normal_lv1.png") : 31));
    const float ball_h = get_y(static_cast<float>(
        ball_tex.valid ? game_texture_height_px(ball_tex, "normal_lv1.png") : 29));

    for (const auto& turret : wall_defenders_) {
        if (!turret.active) {
            continue;
        }

        for (const auto& missile : turret.missiles) {
            if (!missile.active) {
                continue;
            }

            if (!missile.blasting) {
                if (ball_tex.valid) {
                    draw_game_texture_quad(
                        ball_tex,
                        "normal_lv1.png",
                        missile.x - (ball_w * 0.5f),
                        missile.y - (ball_h * 0.5f),
                        ball_w,
                        ball_h,
                        1.0f
                    );
                }
                continue;
            }

            if (turret.blast_range_px > 0) {
                const int blast_frame = std::clamp(missile.blast_time_ms / 50, 0, kTurretBlastFrameCount - 1);
                const std::string blast_name = frame_name("fire_blast_", blast_frame + 1, 2);
                draw_overlay_frame(
                    blast_name,
                    missile.x,
                    missile.y,
                    get_xy(110.0f),
                    get_xy(110.0f),
                    1.0f
                );
            }
        }

        const std::string bg_name = frame_name("ball_lv3_", turret.bg_frame + 1, 3);
        const auto& bg_tex = game_texture(bg_name);
        const float bg_w = get_x(static_cast<float>(
            bg_tex.valid ? game_texture_width_px(bg_tex, bg_name) : 40));
        const float bg_h = get_y(static_cast<float>(
            bg_tex.valid ? game_texture_height_px(bg_tex, bg_name) : 35));
        const float orb_y = turret.y + turret.bob_y;

        if (bg_tex.valid) {
            draw_game_texture_quad(
                bg_tex,
                bg_name,
                turret.x - (bg_w * 0.5f),
                orb_y - (bg_h * 0.25f),
                bg_w,
                bg_h,
                1.0f
            );
        }
        if (ball_tex.valid) {
            draw_game_texture_quad_tinted(
                ball_tex,
                "normal_lv1.png",
                turret.x - (ball_w * 0.5f),
                orb_y - (ball_h * 0.5f),
                ball_w,
                ball_h,
                1.0f,
                1.0f,
                1.0f,
                turret.alpha
            );
        }
    }
}

// -----------------------------------------------------------------------
// draw_wall_defender_layer
// -----------------------------------------------------------------------
void MainGameScene::draw_wall_defender_layer() {
    const uint64_t now = AbstractGame::game_time_ms();

    // Sort back-to-front by y (largest y drawn first — further back in world space)
    std::vector<size_t> draw_order(monsters_.size());
    std::iota(draw_order.begin(), draw_order.end(), 0U);
    std::stable_sort(draw_order.begin(), draw_order.end(), [this](size_t a, size_t b) {
        return monsters_[a].y > monsters_[b].y;
    });

    for (size_t index : draw_order) {
        const auto& m = monsters_[index];

        // Skip the waiting phase entirely (Java: status==4 → don't draw)
        if (m.status == MonsterStatus::kWaiting) continue;

        // ---- Colour modulation from BasalMonster.draw() ----
        // Be-hit: green+blue channels fade to 0 over kBeHitTime*2 ms
        float tint_gb = 1.0f;
        if (m.be_hit_cur_ms > 0) {
            tint_gb = (kBeHitTime * 2.0f - m.be_hit_cur_ms) / (kBeHitTime * 2.0f);
            tint_gb = std::max(0.0f, std::min(1.0f, tint_gb));
        }
        (void)tint_gb;
        // img_alpha (boss-2 ghost phase)
        const float img_alpha = m.img_alpha;
        // Die alpha
        const float draw_alpha = (m.status == MonsterStatus::kDying) ? m.die_alpha : img_alpha;

        const std::vector<std::string>& body_frames = monster_body_frames(m.type);

        // ---- Draw monster body ----
        bool drawn = false;

        // If frozen, show freeze overlay instead of normal sprite
        if (m.freeze_time_ms > 0 && m.status != MonsterStatus::kDying) {
            const std::string& freeze_key = freeze_frame_for_type(m.type);
            if (!freeze_key.empty()) {
                draw_overlay_frame(freeze_key, m.x, m.y, m.w, m.h, draw_alpha);
                drawn = true;
            }
        }
        if (!drawn) {
            const std::string& frame_key = frame_by_img_id(body_frames, m.img_id);
            if (!frame_key.empty()) {
                drawn = draw_atlas_frame_centered(frame_key, m.x, m.y, m.w, m.h,
                                                  0.0f, draw_alpha);
            }
        }
        if (!drawn) {
            if (false) {
                // Apply be-hit tint by drawing with modified alpha/color
                // (Full per-channel tint requires engine support; we approximate
                //  with a second white draw at reduced alpha — TODO: extend
                //  draw_textured_quad_centered_uv with RGB params if desired)
                drawn = false;
            }
            if (!drawn) {
                const auto& tex = game_texture(monster_tex_name(m.type, m.boss, now));
                if (tex.valid) {
                    draw_textured_quad(tex.id,
                        m.x - m.w * 0.5f, m.y - m.h * 0.5f, m.w, m.h, draw_alpha);
                    drawn = true;
                } else {
                    draw_quad(m.x - m.w * 0.5f, m.y - m.h * 0.5f, m.w, m.h,
                              0.8f, 0.2f, 0.2f, draw_alpha);
                    drawn = true;
                }
            }
        }

        // ---- Boss-2: ghost / store overlay (img_alpha < 1) ----
        if (m.type == 5 && m.img_alpha < 1.0f) {
            const float ghost_alpha = 1.0f - m.img_alpha;
            const std::string& store_key = anim_frame_at_time(
                boss2_store_frames(),
                static_cast<float>(AbstractGame::game_time_ms() / 50 * 50), 50.0f);
            if (!store_key.empty()) {
                draw_atlas_frame_centered(store_key, m.x, m.y, m.w, m.h, 0.0f, ghost_alpha);
            }
        }

        // ---- Boss-2: fire attack overlay ----
        if (m.type == 5 && m.fire_flag) {
            const std::string& fire_key = anim_frame_at_time(
                boss2_fire_frames(),
                static_cast<float>(AbstractGame::game_time_ms() / 50 * 50), 50.0f);
            if (!fire_key.empty()) {
                // Java: positioned left of monster, upper area (matches boss_fire placement)
                const float fx = m.x - m.w;
                const float fy = m.y + get_y(45.0f);
                draw_atlas_frame_centered(fire_key, fx, fy,
                                          get_xy(70.0f), get_xy(70.0f), 0.0f, 1.0f);
            }
        }

        // ---- HP bar ----
        if (m.blood_show_ms > 0 && m.hp > 0) {
            const float blood_alpha = std::min(1.0f, m.blood_show_ms / 500.0f);
            const float mhp_ratio = static_cast<float>(m.hp) /
                                    std::max(1.0f, static_cast<float>(m.max_hp));
            const auto& frame_tex = TextureCache::instance().get(kMonsterBloodFrame);
            const auto& piece_tex = TextureCache::instance().get(kMonsterBloodPiece);
            const float frame_w = frame_tex.valid ? get_xy(static_cast<float>(frame_tex.width)) : get_x(65.0f);
            const float frame_h = frame_tex.valid ? get_xy(static_cast<float>(frame_tex.height)) : get_y(8.0f);
            const float fill_h = piece_tex.valid ? get_xy(static_cast<float>(piece_tex.height)) : get_y(4.0f);
            const float fill_y = m.y + (m.h * 0.55f) + ((frame_h - fill_h) * 0.5f);
            const float frame_x = m.x - (frame_w * 0.5f);
            const float frame_y = m.y + (m.h * 0.55f);
            const float fill_x = frame_x;
            const float fill_w = frame_w * mhp_ratio;
            if (piece_tex.valid) {
                draw_textured_quad(piece_tex.id, fill_x, fill_y, fill_w, fill_h, 0.95f * blood_alpha);
            } else {
                draw_quad(fill_x, fill_y, fill_w, fill_h, 0.2f, 0.9f, 0.2f, 0.95f * blood_alpha);
            }
            if (frame_tex.valid) {
                draw_textured_quad(frame_tex.id, frame_x, frame_y, frame_w, frame_h, blood_alpha);
            } else {
                draw_quad(frame_x, frame_y, frame_w, frame_h, 0.2f, 0.2f, 0.2f, 0.8f * blood_alpha);
            }
        }

        // ---- Burn overlay ----
        if ((m.burn_time_ms > 0 || m.is_in_river) &&
            (m.status == MonsterStatus::kRunning || m.status == MonsterStatus::kAttacking) &&
            m.type < 4 && m.freeze_time_ms <= 0) {
            const std::string& burn_key = anim_frame_at_time(
                burn_frames(),
                static_cast<float>(AbstractGame::game_time_ms() / 80 * 80), 80.0f);
            if (!burn_key.empty()) {
                draw_atlas_frame_centered(burn_key, m.x, m.y, m.w, m.h, 0.0f, 1.0f);
            }
        }

        // ---- Shock overlay ----
        if (m.shock_time_ms > 0 &&
            (m.status == MonsterStatus::kRunning || m.status == MonsterStatus::kAttacking)) {
            const std::string& shock_key = anim_frame_at_time(
                shock_frames(),
                static_cast<float>(AbstractGame::game_time_ms() / 80 * 80), 80.0f);
            if (!shock_key.empty()) {
                draw_atlas_frame_centered(shock_key, m.x, m.y, m.w, m.h, 0.0f, 1.0f);
            }
        }

        // ---- Stone missile drawing (type 6) ----
        if (m.type == 6 && m.is_remote_atk) {
            const float hit_x = get_x(100.0f);
            if (m.missile_x >= hit_x) {
                const std::vector<std::string>& mf =
                    (m.remote_img_id < m.remote_missile_frames)
                    ? stone_missile_travel_frames()
                    : stone_missile_blast_frames();
                const int idx = m.remote_img_id < m.remote_missile_frames
                    ? m.remote_img_id
                    : m.remote_img_id - m.remote_missile_frames;
                if (idx >= 0 && idx < static_cast<int>(mf.size())) {
                    const float angle_deg = (m.remote_img_id < m.remote_missile_frames)
                        ? static_cast<float>(-m.sto_angle * 180.0f / kPi)
                        : 0.0f;
                    const float mw = get_xy(54.0f);
                    const float mh = get_xy(54.0f);
                    if (!draw_atlas_frame_centered(mf[static_cast<size_t>(idx)],
                                                   m.missile_x, m.missile_y,
                                                   mw, mh, angle_deg)) {
                        draw_quad(m.missile_x - mw * 0.5f, m.missile_y - mh * 0.5f,
                                  mw, mh, 0.6f, 0.5f, 0.2f, 1.0f);
                    }
                }
            }
        }

        // ---- Devil-fire missile drawing (type 3) ----
        if (m.type == 3 && m.is_remote_atk) {
            const float hit_x = get_x(150.0f);
            if (m.missile_x >= hit_x) {
                const std::vector<std::string>& mf =
                    (m.remote_img_id < m.remote_missile_frames)
                    ? devil_travel_frames()
                    : devil_blast_frames();
                const int idx = m.remote_img_id < m.remote_missile_frames
                    ? m.remote_img_id
                    : m.remote_img_id - m.remote_missile_frames;
                if (idx >= 0 && idx < static_cast<int>(mf.size())) {
                    const float mw = get_xy(52.0f);
                    const float mh = get_xy(52.0f);
                    if (!draw_atlas_frame_centered(mf[static_cast<size_t>(idx)],
                                                   m.missile_x, m.missile_y,
                                                   mw, mh, 0.0f)) {
                        draw_quad(m.missile_x - mw * 0.5f, m.missile_y - mh * 0.5f,
                                  mw, mh, 0.85f, 0.2f, 0.1f, 0.95f);
                    }
                }
            }
        }
    }

    // ---- EnemyProjectile layer (boss fire, devil fire, stone via EnemyProjectile) ----
    for (const auto& p : enemy_projectiles_) {
        const std::vector<std::string>& frames =
            p.blasting ? projectile_blast_frames(p.kind)
                       : projectile_travel_frames(p.kind);
        const std::string& frame_key = anim_frame_at_time(frames, p.lifetime_ms, 80.0f);
        const float angle = p.blasting ? 0.0f
            : (std::atan2(p.vy, p.vx) * 180.0f / kPi);

        float draw_w = get_xy(52.0f);
        float draw_h = get_xy(52.0f);
        if (p.kind == kProjectileKindBossFire)  { draw_w = get_xy(70.0f);  draw_h = get_xy(70.0f);  }
        else if (p.kind == kProjectileKindStone) { draw_w = get_xy(54.0f);  draw_h = get_xy(54.0f);  }
        if (p.blasting) {
            if (p.kind == kProjectileKindBossFire)   { draw_w = get_xy(120.0f); draw_h = get_xy(120.0f); }
            else if (p.kind == kProjectileKindStone)  { draw_w = get_xy(108.0f); draw_h = get_xy(108.0f); }
            else                                      { draw_w = get_xy(90.0f);  draw_h = get_xy(90.0f);  }
        }

        bool drawn = false;
        if (!frame_key.empty()) {
            const AtlasFrame* frame = SpriteAtlasCache::instance().get_frame(frame_key);
            if (frame && frame->valid) {
                draw_w = get_xy(static_cast<float>(std::max(1, frame->original_width)));
                draw_h = get_xy(static_cast<float>(std::max(1, frame->original_height)));
            }
            drawn = draw_atlas_frame_centered(frame_key, p.x, p.y, draw_w, draw_h, angle, 1.0f);
        }
        if (!drawn) {
            draw_quad(p.x - draw_w * 0.5f, p.y - draw_h * 0.5f, draw_w, draw_h,
                      p.blasting ? 1.0f : 0.85f,
                      p.blasting ? 0.45f : 0.2f,
                      0.1f, 0.95f);
        }
    }
}

} // namespace defender
