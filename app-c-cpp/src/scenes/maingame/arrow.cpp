#include "scenes/main_game_scene.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>

#include "engine/abstract_game.hpp"
#include "engine/game_texture_specs.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/audio_manager.hpp"
#include "game/bow_data.hpp"
#include "game/item_param.hpp"
#include "game/sounds.hpp"

namespace defender {
namespace {

constexpr float kPi = 3.14159265358979323846f;

float deg_to_rad(float deg) {
    return deg * kPi / 180.0f;
}

const TextureHandle& game_texture(const std::string& file_name) {
    return TextureCache::instance().get("assets/imgs_480_800/game/" + file_name);
}

int item_level(const MainGameScene* scene, int type) {
    return scene->is_rep() ? ItemParam::get_level(type, false) : ItemParam::get_level(type);
}

std::string arrow_texture_name(int bow_level) {
    if (bow_level <= 0) {
        return "arrow_normal.png";
    }
    if (bow_level >= BowData::FINAL) {
        return "arrow_final.png";
    }
    if (bow_level <= 9) {
        return "arrow_pow_0" + std::to_string(bow_level) + ".png";
    }
    if (bow_level <= 18) {
        return "arrow_agi_0" + std::to_string(bow_level - 9) + ".png";
    }
    return "arrow_mul_0" + std::to_string(bow_level - 18) + ".png";
}

}

void MainGameScene::shoot_volley(float target_x, float target_y) {
    const float dy = target_y - bow_y_;
    const float dx = std::max(get_x(1.0f), target_x - bow_x_);
    target_angle_deg_ = std::atan2(dy, dx) * 180.0f / kPi;

    const float last_span_ms = static_cast<float>(AbstractGame::last_span_ms());
    if (last_span_ms < 50.0f && !is_rep()) {
        bow_angle_deg_ += ((target_angle_deg_ - bow_angle_deg_) * last_span_ms) / 50.0f;
    } else {
        bow_angle_deg_ = target_angle_deg_;
    }

    if (std::abs(bow_angle_deg_ - target_angle_deg_) >= 5.0f) {
        return;
    }

    bow_angle_deg_ = target_angle_deg_;
    if (bow_cd_left_ms_ > 0 && !is_rep()) {
        return;
    }
    bow_cd_left_ms_ = bow_cd_ms_;
    AudioManager::instance().play_sound(Sounds::ARROW_SHOT_SND);

    auto shoot_single = [this](float angle_deg) {
        const float rad = deg_to_rad(angle_deg);
        ArrowEntity arrow;
        arrow.x = bow_x_;
        arrow.y = bow_y_;
        arrow.vx = std::cos(rad) * arrow_speed_;
        arrow.vy = std::sin(rad) * arrow_speed_;
        arrow.power = arrow_power_;
        arrows_.push_back(arrow);
    };

    switch (arrow_num_) {
    case 1:
        shoot_single(bow_angle_deg_);
        break;
    case 2:
        shoot_single(bow_angle_deg_ + 1.5f);
        shoot_single(bow_angle_deg_ - 1.5f);
        break;
    case 3:
        shoot_single(bow_angle_deg_ + 3.0f);
        shoot_single(bow_angle_deg_);
        shoot_single(bow_angle_deg_ - 3.0f);
        break;
    case 4:
        shoot_single(bow_angle_deg_ + 1.5f);
        shoot_single(bow_angle_deg_ - 1.5f);
        shoot_single(bow_angle_deg_ + 4.5f);
        shoot_single(bow_angle_deg_ - 4.5f);
        break;
    default:
        shoot_single(bow_angle_deg_ + 3.0f);
        shoot_single(bow_angle_deg_ - 3.0f);
        shoot_single(bow_angle_deg_);
        shoot_single(bow_angle_deg_ + 6.0f);
        shoot_single(bow_angle_deg_ - 6.0f);
        break;
    }
}

void MainGameScene::update_arrows(float dt_ms) {
    const float dt = dt_ms / 1000.0f;
    const float hit_probe = get_x(85.0f * 0.9f);
    for (size_t i = 0; i < arrows_.size();) {
        auto& a = arrows_[i];
        a.x += a.vx * dt;
        a.y += a.vy * dt;

        float dir_x = a.vx;
        float dir_y = a.vy;
        const float len = std::sqrt((dir_x * dir_x) + (dir_y * dir_y));
        if (len > 0.001f) {
            dir_x /= len;
            dir_y /= len;
        } else {
            dir_x = 1.0f;
            dir_y = 0.0f;
        }
        const float tip_x = a.x + (dir_x * hit_probe);
        const float tip_y = a.y + (dir_y * hit_probe);

        bool removed = false;
        for (size_t m = 0; m < monsters_.size(); ++m) {
            const auto& mon = monsters_[m];
            const float cx = mon.x;
            const float cy = mon.y;
            const float center_rx = std::max(get_x(10.0f), mon.w * 0.18f);
            const float center_ry = std::max(get_y(10.0f), mon.h * 0.18f);
            if (std::abs(tip_x - cx) <= center_rx && std::abs(tip_y - cy) <= center_ry) {
                if (monster_be_hit(m, a.power, 2)) {
                    arrows_.erase(arrows_.begin() + static_cast<std::ptrdiff_t>(i));
                    removed = true;
                    break;
                }
            }
        }

        if (removed) {
            continue;
        }

        if (a.x > screen_width || a.y < 0.0f || a.y > screen_height) {
            arrows_.erase(arrows_.begin() + static_cast<std::ptrdiff_t>(i));
            continue;
        }

        ++i;
    }
}

void MainGameScene::draw_arrow_layer() {
    const int bow_level = std::max(0, item_level(this, ItemParam::BOW_EQUIP));
    const std::string arrow_name = arrow_texture_name(bow_level);
    const auto& arrow_tex = game_texture(arrow_name);
    const float arrow_draw_w = get_x(static_cast<float>(
        arrow_tex.valid ? game_texture_width_px(arrow_tex, arrow_name) : 85));
    const float arrow_draw_h = get_y(static_cast<float>(
        arrow_tex.valid ? game_texture_height_px(arrow_tex, arrow_name) : 9));
    const float pivot_x = arrow_draw_w * 0.1f;
    const float pivot_y = arrow_draw_h * 0.5f;
    for (const auto& a : arrows_) {
        const float angle = std::atan2(a.vy, a.vx) * 180.0f / kPi;
        if (arrow_tex.valid) {
            draw_game_texture_quad_pivoted(
                arrow_tex,
                arrow_name,
                a.x,
                a.y,
                arrow_draw_w,
                arrow_draw_h,
                pivot_x,
                pivot_y,
                angle,
                1.0f
            );
        } else {
            draw_quad(
                a.x - pivot_x,
                a.y - pivot_y,
                arrow_draw_w,
                arrow_draw_h,
                0.95f,
                0.92f,
                0.35f,
                1.0f
            );
        }
    }
}

}
