// magic.cpp
// Ported from Magic.java + BasalMagic behaviour.
// Preserves: cast patterns (multi-shot, scatter, cross, column),
// freeze/shock/burn durations from speTime, power bonuses from Param,
// sorted insert order, recycle pool, range-ring display.

#include "scenes/main_game_scene.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "engine/abstract_game.hpp"
#include "engine/render_utils.hpp"
#include "engine/sprite_atlas_cache.hpp"
#include "engine/texture_cache.hpp"
#include "game/achv_mng.hpp"
#include "game/audio_manager.hpp"
#include "game/param.hpp"
#include "game/sounds.hpp"

namespace defender {
namespace {

constexpr int kEffectCritical = 1;



constexpr int kMagicFireBasic  = 1;
constexpr int kMagicFireBurst  = 2;
constexpr int kMagicFireRain   = 3;
constexpr int kMagicIceBasic   = 4;
constexpr int kMagicIceCross   = 5;
constexpr int kMagicIceBliz    = 6;
constexpr int kMagicLightBasic = 7;
constexpr int kMagicLightCol   = 8;
constexpr int kMagicLightStorm = 9;

constexpr float kMagicFrameMs     = 80.0f;

// Screen dimensions in Java reference units (800x480)
constexpr float kJavaScreenW = 800.0f;
constexpr float kJavaScreenH = 480.0f;

float dist2(float ax, float ay, float bx, float by) {
    const float dx = ax - bx;
    const float dy = ay - by;
    return dx * dx + dy * dy;
}

float atlas_draw_size_for(const std::string& key, MainGameScene* scene, float fallback_units, float scale = 1.0f) {
    const AtlasFrame* frame = SpriteAtlasCache::instance().get_frame(key);
    if (!frame || !frame->valid) {
        return scene->get_xy(fallback_units) * scale;
    }
    const float base = static_cast<float>(std::max(frame->original_width, frame->original_height));
    return scene->get_xy(base) * scale;
}

std::string magic_travel_frame(int magic_type, int frame_idx) {
    auto make = [](const char* prefix, int idx, int pad) -> std::string {
        std::string n = std::to_string(idx + 1);
        while (static_cast<int>(n.size()) < pad) n = "0" + n;
        return std::string(prefix) + n + ".png";
    };
    if (magic_type <= kMagicFireRain)  return make("fire_ball_",    frame_idx, 4);
    if (magic_type <= kMagicIceBliz)   return make("ice_piton_",    frame_idx, 3);
    return                                    make("light_strike_", frame_idx, 1);
}

std::string magic_blast_frame(int magic_type, int frame_idx) {
    auto make = [](const char* prefix, int idx, int pad) -> std::string {
        std::string n = std::to_string(idx + 1);
        while (static_cast<int>(n.size()) < pad) n = "0" + n;
        return std::string(prefix) + n + ".png";
    };
    if (magic_type <= kMagicFireRain)  return make("fire_blast_",  frame_idx, 2);
    if (magic_type <= kMagicIceBliz)   return make("ice_piton_",   frame_idx + 4, 3);
    return                                    magic_travel_frame(magic_type, frame_idx);
}

int travel_frame_count(int magic_type) {
    if (magic_type <= kMagicFireRain)  return 4;
    if (magic_type <= kMagicIceBliz)   return 4;
    return 6;
}

int blast_frame_count(int magic_type) {
    if (magic_type <= kMagicFireRain)  return 10;
    if (magic_type <= kMagicIceBliz)   return 4;
    return 6;
}

float magic_draw_size(int magic_type, MainGameScene* s) {
    if (magic_type <= kMagicFireRain)  return atlas_draw_size_for("fire_ball_0001.png", s, 60.0f, 1.0f);
    if (magic_type <= kMagicIceBliz)   return atlas_draw_size_for("ice_piton_001.png", s, 72.0f, 1.0f);
    return atlas_draw_size_for("light_strike_1.png", s, 56.0f, 1.0f);
}

float magic_blast_size(int magic_type, MainGameScene* s) {
    if (magic_type <= kMagicFireRain)  return atlas_draw_size_for("fire_blast_01.png", s, 110.0f, 1.5f);
    if (magic_type <= kMagicIceBliz)   return atlas_draw_size_for("ice_piton_005.png", s, 130.0f, 1.0f);
    return atlas_draw_size_for("light_strike_1.png", s, 100.0f, 1.15f);
}

float magic_hit_radius(int magic_type, MainGameScene* s) {
    if (magic_type <= kMagicFireRain)  return s->get_xy(65.0f);
    if (magic_type <= kMagicIceBliz)   return s->get_xy(80.0f);
    return s->get_xy(60.0f);
}

bool draw_atlas_frame_centered(const std::string& key,
                               float cx, float cy, float w, float h,
                               float angle_deg = 0.0f, float alpha = 1.0f) {
    const AtlasFrame* frame = SpriteAtlasCache::instance().get_frame(key);
    if (!frame || !frame->valid || frame->texture_id == 0) return false;
    const float src_w = static_cast<float>(std::max(1, frame->original_width));
    const float src_h = static_cast<float>(std::max(1, frame->original_height));
    draw_textured_quad_centered_uv(
        frame->texture_id,
        cx + frame->offset_x * (w / src_w),
        cy + frame->offset_y * (h / src_h),
        w * (static_cast<float>(frame->width)  / src_w),
        h * (static_cast<float>(frame->height) / src_h),
        frame->u0, frame->v0, frame->u1, frame->v1,
        angle_deg, alpha);
    return true;
}

} // namespace

// ---------------------------------------------------------------------------
// apply_aoe_damage
// ---------------------------------------------------------------------------
void MainGameScene::apply_aoe_damage(float x, float y, float radius,
                                     int damage, int apply_slow_ms) {
    const float r2 = radius * radius;
    for (size_t i = 0; i < monsters_.size();) {
        auto& mon = monsters_[i];
        if (dist2(mon.x, mon.y, x, y) <= r2) {
            mon.hp            -= damage;
            mon.be_hit_cur_ms  = 200;
            mon.blood_show_ms  = 1000;
            if (apply_slow_ms > 0) {
                mon.freeze_time_ms = std::max(mon.freeze_time_ms, apply_slow_ms);
            }
            if (mon.hp <= 0) {
                mon.hp = 0;
                if (mon.status != MainGameScene::MonsterStatus::kDying &&
                    mon.status != MainGameScene::MonsterStatus::kRemove) {
                    mon.status = MainGameScene::MonsterStatus::kDying;
                    mon.img_time_ms = 0;
                    mon.die_alpha = 1.0f;
                }
            }
        }
        ++i;
    }
}

// ---------------------------------------------------------------------------
// magic_add — sorted insert by Y descending (Java addSequenceMagic)
// ---------------------------------------------------------------------------
void MainGameScene::magic_add(int magic_type, int power, int spe_time,
                               float x, float y, int lag_ms) {
    MagicInstance inst;
    inst.magic_type  = magic_type;
    inst.power       = power;
    inst.spe_time    = spe_time;
    inst.target_x    = x;
    inst.target_y    = y;
    inst.lag_ms      = lag_ms;
    inst.elapsed_ms  = 0;
    inst.active      = (lag_ms <= 0);
    inst.finished    = false;
    inst.blasting    = false;
    inst.blast_ms    = 0;
    inst.img_id      = 0;
    inst.img_time_ms = 0;
    inst.damage_applied = false;
    if (magic_type <= kMagicFireRain) {
        inst.x = (x - screen_height) + y;
        inst.y = screen_height;
    } else {
        inst.x = x;
        inst.y = y;
    }

    // Higher Y → drawn first (Java: if basalMagic.getY() > list[i].getY() → insert before)
    auto it = magic_using_.begin();
    while (it != magic_using_.end() && inst.y <= it->y) ++it;
    magic_using_.insert(it, inst);
}

// ---------------------------------------------------------------------------
// magic_show_range / magic_update_range / magic_cancel
// ---------------------------------------------------------------------------
void MainGameScene::magic_show_range(float x, float y, int range_units) {
    if (!magic_show_range_) {
        magic_show_range_ = true;
        magic_range_px_   = static_cast<int>(get_xy(static_cast<float>(range_units)));
        magic_target_x_   = x;
        magic_target_y_   = y;
    }
}

void MainGameScene::magic_update_range(float x, float y) {
    if (magic_show_range_) {
        magic_target_x_ = x;
        magic_target_y_ = y;
    }
}

void MainGameScene::magic_cancel() {
    magic_show_range_ = false;
}

// ---------------------------------------------------------------------------
// magic_cast — full cast pattern table from Java Magic.castMagic()
// ---------------------------------------------------------------------------
void MainGameScene::magic_cast(int magic_type, int power, int spe_time,
                                float x, float y) {
    // Param power bonuses (Java: power += extraFire*power/100 etc.)
    switch (magic_type) {
    case kMagicFireBasic: case kMagicFireBurst: case kMagicFireRain:
        Param::cast_fire += 1;
        AchvMng::check_achv_in_game(5);
        power = static_cast<int>(power + (Param::extra_fire * power) / 100.0f);
        break;
    case kMagicIceBasic: case kMagicIceCross: case kMagicIceBliz:
        Param::cast_ice += 1;
        AchvMng::check_achv_in_game(6);
        power = static_cast<int>(power + (Param::extra_ice * power) / 100.0f);
        break;
    case kMagicLightBasic: case kMagicLightCol: case kMagicLightStorm:
        Param::cast_light += 1;
        AchvMng::check_achv_in_game(7);
        power = static_cast<int>(power + (Param::extra_light * power) / 100.0f);
        break;
    default: break;
    }

    if (!magic_show_range_) return;

    switch (magic_type) {
    // Single shot
    case kMagicFireBasic:
    case kMagicIceBasic:
    case kMagicLightBasic:
        magic_add(magic_type, power, spe_time, x, y, 0);
        break;

    // Fire burst: 5 shots 600 ms apart, scattered (Java COVER_LOGO=140 → nextInt(140))
    case kMagicFireBurst:
        magic_add(magic_type, power, spe_time, x, y, 0);
        for (int i = 1; i <= 4; ++i) {
            const float ox = get_x(static_cast<float>(70 - static_cast<int>(form_rng_() % 140)));
            const float oy = get_y(static_cast<float>(70 - static_cast<int>(form_rng_() % 140)));
            magic_add(magic_type, power, spe_time, x + ox, y + oy, i * 600);
        }
        break;

    // Fire rain / lightning storm: 40 random positions, 50 ms apart
    case kMagicFireRain:
    case kMagicLightStorm:
        for (int i = 0; i < 40; ++i) {
            const float rx = get_x(kJavaScreenW * static_cast<float>(form_rng_() % 80 + 20) / 105.0f);
            const float ry = get_y(kJavaScreenH * static_cast<float>(form_rng_() % 50 + 10) / 60.0f);
            magic_add(magic_type, power, spe_time, rx, ry, i * 50);
        }
        break;

    // Ice cross: centre + 4 cardinal arms, 150 ms stagger
    // Java used GLTextures.QUANJINIAO_RUN_LAN_0001 = 450 as the 3rd lag value
    case kMagicIceCross:
        magic_add(magic_type, power, spe_time, x,               y,   0);
        magic_add(magic_type, power, spe_time, x - get_x(130.0f), y, 150);
        magic_add(magic_type, power, spe_time, x + get_x(130.0f), y, 450);
        magic_add(magic_type, power, spe_time, x, y - get_y(130.0f), 600);
        magic_add(magic_type, power, spe_time, x, y + get_y(130.0f), 300);
        break;

    // Ice blizzard: 40 random positions, 30 ms apart
    case kMagicIceBliz:
        for (int i = 0; i < 40; ++i) {
            const float rx = get_x(kJavaScreenW * static_cast<float>(form_rng_() % 80 + 20) / 105.0f);
            const float ry = get_y(kJavaScreenH * static_cast<float>(form_rng_() % 50 + 10) / 60.0f);
            magic_add(magic_type, power, spe_time, rx, ry, i * 30);
        }
        break;

    // Lightning column: 5 shots descending Y=400→120 in Java units, 150 ms apart
    case kMagicLightCol:
        magic_add(magic_type, power, spe_time, x, get_y(400.0f),   0);
        magic_add(magic_type, power, spe_time, x, get_y(330.0f), 150);
        magic_add(magic_type, power, spe_time, x, get_y(260.0f), 300);
        magic_add(magic_type, power, spe_time, x, get_y(190.0f), 450);
        magic_add(magic_type, power, spe_time, x, get_y(120.0f), 600);
        break;

    default: break;
    }

    magic_show_range_ = false;

    // Java: Param.spellData[magicType-1]++
    if (magic_type >= 1 && magic_type <= static_cast<int>(Param::spell_data.size())) {
        Param::spell_data[static_cast<size_t>(magic_type - 1)] += 1;
    }
}

// ---------------------------------------------------------------------------
// magic_apply_hit — damage + status effect on contact
// Matches BasalMonster.specialEffect() routing
// ---------------------------------------------------------------------------
void MainGameScene::magic_apply_hit(const MagicInstance& inst) {
    const float hit_r = magic_hit_radius(inst.magic_type, this);
    const float r2    = hit_r * hit_r;

    for (size_t i = 0; i < monsters_.size();) {
        const auto& mon = monsters_[i];
        if (dist2(mon.x, mon.y, inst.x, inst.y) > r2) {
            ++i;
            continue;
        }
        if (monster_be_hit(i, inst.power, 1)) {
            monster_apply_magic_effect(i, inst.magic_type, inst.power, inst.spe_time);
        }
        ++i;
    }
}

// ---------------------------------------------------------------------------
// magic_update — called every frame from MainGameScene::update()
// ---------------------------------------------------------------------------
void MainGameScene::magic_update(float dt_ms) {
    const int dt = static_cast<int>(dt_ms);

    for (size_t j = 0; j < magic_using_.size();) {
        MagicInstance& inst = magic_using_[j];

        // Count down lag before activating
        if (!inst.active) {
            inst.lag_ms -= dt;
            if (inst.lag_ms <= 0) {
                inst.active = true;
            } else {
                ++j; continue;
            }
        }

        if (inst.blasting) {
            inst.blast_ms    += dt;
            inst.img_time_ms += dt;
            const int total_blast_ms = blast_frame_count(inst.magic_type) *
                                       static_cast<int>(kMagicFrameMs);
            if (inst.blast_ms >= total_blast_ms) {
                inst.finished = true;
            }
        } else {
            inst.elapsed_ms  += dt;
            inst.img_time_ms += dt;
            inst.img_id = (inst.img_time_ms / static_cast<int>(kMagicFrameMs))
                          % travel_frame_count(inst.magic_type);

            // One full travel cycle → hit lands
            const int full_travel_ms = travel_frame_count(inst.magic_type) *
                                       static_cast<int>(kMagicFrameMs);
            if (inst.elapsed_ms >= full_travel_ms) {
                magic_apply_hit(inst);
                inst.blasting    = true;
                inst.blast_ms    = 0;
                inst.img_time_ms = 0;
                inst.img_id      = 0;
            }
        }

        if (inst.finished) {
            magic_using_.erase(magic_using_.begin() +
                static_cast<std::ptrdiff_t>(j));
            continue;
        }
        ++j;
    }
}

void MainGameScene::magic_update_java(float dt_ms) {
    const int dt = static_cast<int>(dt_ms);

    for (size_t j = 0; j < magic_using_.size();) {
        MagicInstance& inst = magic_using_[j];

        if (!inst.active) {
            inst.lag_ms -= dt;
            if (inst.lag_ms <= 0) {
                inst.active = true;
            } else {
                ++j;
                continue;
            }
        }

        inst.elapsed_ms += dt;
        inst.img_time_ms = inst.elapsed_ms;

        if (inst.magic_type <= kMagicFireRain) {
            if (!inst.blasting) {
                inst.x += (get_x(1000.0f) * dt_ms) / 1000.0f;
                inst.y -= (get_y(1000.0f) * dt_ms) / 1000.0f;
                inst.img_id = (inst.elapsed_ms / static_cast<int>(kMagicFrameMs)) % travel_frame_count(inst.magic_type);
                if (inst.x >= inst.target_x || inst.y <= inst.target_y) {
                    inst.x = inst.target_x;
                    inst.y = inst.target_y;
                    if (!inst.damage_applied) {
                        magic_apply_hit(inst);
                        inst.damage_applied = true;
                    }
                    inst.blasting = true;
                    inst.blast_ms = 0;
                    inst.elapsed_ms = 0;
                    inst.img_time_ms = 0;
                    inst.img_id = 0;
                }
            } else {
                inst.blast_ms = inst.elapsed_ms;
                inst.img_id = inst.elapsed_ms / static_cast<int>(kMagicFrameMs);
                if (inst.img_id > blast_frame_count(inst.magic_type) - 1) {
                    inst.finished = true;
                }
            }
        } else if (inst.magic_type <= kMagicIceBliz) {
            if (!inst.blasting) {
                inst.img_id = (inst.elapsed_ms * 2) / static_cast<int>(kMagicFrameMs);
                if (inst.img_id > travel_frame_count(inst.magic_type) - 1) {
                    inst.img_id = travel_frame_count(inst.magic_type) - 1;
                }
                if (!inst.damage_applied) {
                    magic_apply_hit(inst);
                    inst.damage_applied = true;
                }
                if (inst.elapsed_ms > 1000) {
                    inst.blasting = true;
                    inst.blast_ms = 0;
                    inst.elapsed_ms = 0;
                    inst.img_time_ms = 0;
                    inst.img_id = 0;
                }
            } else {
                inst.blast_ms = inst.elapsed_ms;
                inst.img_id = inst.elapsed_ms / static_cast<int>(kMagicFrameMs);
                if (inst.img_id > blast_frame_count(inst.magic_type) - 1) {
                    inst.finished = true;
                }
            }
        } else {
            inst.img_id = inst.elapsed_ms / static_cast<int>(kMagicFrameMs);
            if (inst.img_id >= 2 && !inst.damage_applied) {
                magic_apply_hit(inst);
                inst.damage_applied = true;
            }
            if (inst.img_id > travel_frame_count(inst.magic_type) - 1) {
                inst.finished = true;
            }
        }

        if (inst.finished) {
            magic_using_.erase(magic_using_.begin() + static_cast<std::ptrdiff_t>(j));
            continue;
        }
        ++j;
    }
}

// ---------------------------------------------------------------------------
// magic_draw — range ring + all active instances
// ---------------------------------------------------------------------------
void MainGameScene::magic_draw() {
    if (magic_show_range_ && magic_range_px_ > 0) {
        const float ring_size = static_cast<float>(magic_range_px_) * 2.0f;
        if (!draw_atlas_frame_centered("magic_ring.png",
                                       magic_target_x_,
                                       magic_target_y_,
                                       ring_size,
                                       ring_size,
                                       0.0f,
                                       0.82f)) {
            const auto& ring_tex = TextureCache::instance().get(
                "assets/imgs_480_800/game/magic_ring.png");
            if (ring_tex.valid) {
                draw_textured_quad(ring_tex.id,
                    magic_target_x_ - ring_size * 0.5f,
                    magic_target_y_ - ring_size * 0.5f,
                    ring_size,
                    ring_size,
                    0.82f);
            }
        }
        const float pulse = 0.9f + (0.12f * std::sin(static_cast<float>(AbstractGame::game_time_ms()) / 90.0f));
        const auto& flash_tex = TextureCache::instance().get("assets/imgs_480_800/game/magic_button_flash.png");
        const float flash_w = get_xy(64.0f) * pulse;
        const float flash_h = get_xy(64.0f) * pulse;
        if (flash_tex.valid) {
            draw_textured_quad(
                flash_tex.id,
                magic_target_x_ - (flash_w * 0.5f),
                magic_target_y_ - (flash_h * 0.5f),
                flash_w,
                flash_h,
                0.82f
            );
        } else {
            draw_quad(
                magic_target_x_ - (flash_w * 0.5f),
                magic_target_y_ - (flash_h * 0.5f),
                flash_w,
                flash_h,
                0.95f,
                0.9f,
                0.45f,
                0.55f
            );
        }
    }

    for (const auto& inst : magic_using_) {
        if (!inst.active) continue;

        const float sz_travel = magic_draw_size(inst.magic_type, this);
        const float sz_blast  = magic_blast_size(inst.magic_type, this);

        if (inst.blasting) {
            const int fidx = std::min(
                inst.img_time_ms / static_cast<int>(kMagicFrameMs),
                blast_frame_count(inst.magic_type) - 1);
            const std::string key = magic_blast_frame(inst.magic_type, fidx);
            if (!draw_atlas_frame_centered(key, inst.x, inst.y, sz_blast, sz_blast)) {
                const float hb = sz_blast * 0.5f;
                const bool  is_fire  = inst.magic_type <= kMagicFireRain;
                const bool  is_ice   = inst.magic_type >= kMagicIceBasic &&
                                       inst.magic_type <= kMagicIceBliz;
                draw_quad(inst.x - hb, inst.y - hb, sz_blast, sz_blast,
                          is_fire ? 1.0f : 0.3f,
                          is_ice  ? 0.8f : 0.3f,
                          (!is_fire && !is_ice) ? 1.0f : 0.1f,
                          0.85f);
            }
        } else {
            const std::string key = magic_travel_frame(inst.magic_type, inst.img_id);
            if (!draw_atlas_frame_centered(key, inst.x, inst.y, sz_travel, sz_travel)) {
                const float ht = sz_travel * 0.5f;
                const bool  is_fire = inst.magic_type <= kMagicFireRain;
                const bool  is_ice  = inst.magic_type >= kMagicIceBasic &&
                                      inst.magic_type <= kMagicIceBliz;
                draw_quad(inst.x - ht, inst.y - ht, sz_travel, sz_travel,
                          is_fire ? 1.0f : 0.2f,
                          is_ice  ? 0.9f : 0.2f,
                          (!is_fire && !is_ice) ? 0.95f : 0.1f,
                          0.9f);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// update_effects
// ---------------------------------------------------------------------------
void MainGameScene::update_effects(float dt_ms) {
    for (size_t i = 0; i < effects_.size();) {
        effects_[i].ttl_ms -= dt_ms;
        if (effects_[i].kind == kEffectCritical) {
            effects_[i].x += effects_[i].vx * (dt_ms / 1000.0f);
            effects_[i].y += effects_[i].vy * (dt_ms / 1000.0f);
        } else {
            effects_[i].radius += get_x(40.0f) * (dt_ms / 1000.0f);
        }
        if (effects_[i].ttl_ms <= 0.0f) {
            effects_.erase(effects_.begin() + static_cast<std::ptrdiff_t>(i));
        } else {
            ++i;
        }
    }
}

// ---------------------------------------------------------------------------
// Spell cast entry points
// ---------------------------------------------------------------------------
void MainGameScene::cast_spell_fire(float x, float y) {
    auto& slot = magic_slots_[0];
    if (slot.magic_type == 0 || slot.cd_left_ms > 0 || wall_mana_ < slot.mana_cost) {
        return;
    }
    wall_mana_ -= slot.mana_cost;
    slot.cd_left_ms = slot.total_cd_ms;
    magic_show_range_ = true;
    magic_target_x_   = x;
    magic_target_y_   = y;
    magic_cast(slot.magic_type, slot.power, slot.spe_time, x, y);
    AudioManager::instance().play_sound(Sounds::MAGIC_FIRE_1);
    effects_.push_back({x, y, get_xy(static_cast<float>(slot.range_units) * 0.5f),
                        300.0f, 1.0f, 0.35f, 0.12f});
}

void MainGameScene::cast_spell_ice(float x, float y) {
    auto& slot = magic_slots_[1];
    if (slot.magic_type == 0 || slot.cd_left_ms > 0 || wall_mana_ < slot.mana_cost) {
        return;
    }
    wall_mana_ -= slot.mana_cost;
    slot.cd_left_ms = slot.total_cd_ms;
    magic_show_range_ = true;
    magic_target_x_   = x;
    magic_target_y_   = y;
    magic_cast(slot.magic_type, slot.power, slot.spe_time, x, y);
    AudioManager::instance().play_sound(Sounds::MAGIC_ICE_1);
    effects_.push_back({x, y, get_xy(static_cast<float>(slot.range_units) * 0.55f),
                        420.0f, 0.35f, 0.65f, 1.0f});
}

void MainGameScene::cast_spell_lightning(float x, float y) {
    auto& slot = magic_slots_[2];
    if (slot.magic_type == 0 || slot.cd_left_ms > 0 || wall_mana_ < slot.mana_cost) {
        return;
    }
    wall_mana_ -= slot.mana_cost;
    slot.cd_left_ms = slot.total_cd_ms;
    magic_show_range_ = true;
    magic_target_x_   = x;
    magic_target_y_   = y;
    magic_cast(slot.magic_type, slot.power, slot.spe_time, x, y);
    AudioManager::instance().play_sound(Sounds::LIGHTNING_1_SND);
}

// ---------------------------------------------------------------------------
// draw_effect_layer
// ---------------------------------------------------------------------------
void MainGameScene::draw_effect_layer() {
    magic_draw();
    for (const auto& fx : effects_) {
        if (fx.kind == kEffectCritical) {
            const auto& tex = TextureCache::instance().get("assets/imgs_480_800/always/critical_hit.png");
            const float show_ms = 600.0f;
            const float progress = std::clamp((show_ms - fx.ttl_ms) / 150.0f, 0.0f, 1.0f);
            const float alpha = std::clamp(fx.ttl_ms / 200.0f, 0.0f, 1.0f);
            const float draw_w = std::max(get_x(65.0f), fx.w) * progress;
            const float draw_h = std::max(get_y(32.0f), fx.h) * progress;
            if (tex.valid) {
                draw_textured_quad(
                    tex.id,
                    fx.x - (draw_w * 0.5f),
                    fx.y - (draw_h * 0.5f),
                    draw_w,
                    draw_h,
                    alpha
                );
            } else {
                draw_quad(fx.x - (draw_w * 0.5f), fx.y - (draw_h * 0.5f), draw_w, draw_h, 1.0f, 0.2f, 0.2f, alpha);
            }
            continue;
        }
        const float alpha = std::max(0.0f, fx.ttl_ms / 420.0f);
        draw_quad(fx.x - fx.radius, fx.y - fx.radius,
                  fx.radius * 2.0f, fx.radius * 2.0f,
                  fx.r, fx.g, fx.b, alpha * 0.45f);
    }
}

} // namespace defender
