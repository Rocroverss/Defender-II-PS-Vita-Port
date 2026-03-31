#include "scenes/main_game_scene.hpp"

#include <algorithm>
#include <string>

#include "engine/game_texture_specs.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/item_param.hpp"
#include "game/param.hpp"

namespace defender {
namespace {

const TextureHandle& game_texture(const std::string& file_name) {
    return TextureCache::instance().get("assets/imgs_480_800/game/" + file_name);
}

int item_level(const MainGameScene* scene, int type) {
    return scene->is_rep() ? ItemParam::get_level(type, false) : ItemParam::get_level(type);
}

const char* wall_front_texture_name(int wall_level) {
    int tier = wall_level / 4;
    if (tier < 0) {
        tier = 0;
    }
    if (tier > 2) {
        tier = 2;
    }
    static const char* kNames[] = {
        "bg_lv1.jpg",
        "bg_lv2.jpg",
        "bg_lv3.jpg"
    };
    return kNames[tier];
}

}

void MainGameScene::update_wall_state(float dt_ms) {
    if (bow_cd_left_ms_ > 0) {
        bow_cd_left_ms_ -= static_cast<int>(dt_ms);
        if (bow_cd_left_ms_ < 0) {
            bow_cd_left_ms_ = 0;
        }
    }
    for (auto& magic_slot : magic_slots_) {
        if (magic_slot.cd_left_ms > 0) {
            magic_slot.cd_left_ms -= static_cast<int>(dt_ms);
            if (magic_slot.cd_left_ms < 0) {
                magic_slot.cd_left_ms = 0;
            }
        }
    }

    mana_accum_ += mana_regen_per_sec_ * (dt_ms / 1000.0f);
    const int mana_gain = static_cast<int>(mana_accum_);
    if (mana_gain > 0) {
        wall_mana_ = std::min(wall_max_mana_, wall_mana_ + mana_gain);
        mana_accum_ -= mana_gain;
    }

    if (wall_hp_ <= 0) {
        wall_hp_ = 0;
        Param::life_percent = 0;
        is_game_over_ = true;
    }
}

void MainGameScene::draw_wall_layer(float hp_ratio) {
    const int wall_level = std::max(0, item_level(this, ItemParam::WALL));
    const std::string wall_front_name = wall_front_texture_name(wall_level);
    const auto& wall_front_tex = game_texture(wall_front_name);
    const float wall_draw_w = get_x(
        static_cast<float>(game_texture_width_px(wall_front_tex, wall_front_name)));
    if (wall_front_tex.valid) {
        draw_game_texture_quad(
            wall_front_tex,
            wall_front_name,
            0.0f,
            0.0f,
            wall_draw_w,
            screen_height,
            1.0f
        );
    } else {
        const float wall_w = get_x(32.0f);
        const float wall_h = get_y(290.0f);
        draw_quad(wall_x_ - (wall_w * 0.5f), get_y(95.0f), wall_w, wall_h, 0.30f, 0.30f, 0.35f, 1.0f);
    }

    const char* broken_name = nullptr;
    if (hp_ratio < 0.3f) {
        broken_name = "wall_broken_2.png";
    } else if (hp_ratio < 0.6f) {
        broken_name = "wall_broken_1.png";
    }

    if (broken_name != nullptr) {
        const auto& wall_broken_tex = game_texture(broken_name);
        if (wall_broken_tex.valid) {
            draw_game_texture_quad(
                wall_broken_tex,
                broken_name,
                0.0f,
                0.0f,
                wall_draw_w,
                screen_height,
                1.0f
            );
        }
    }
}

}
