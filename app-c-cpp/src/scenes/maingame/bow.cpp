#include "scenes/main_game_scene.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include "engine/game_texture_specs.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/bow_data.hpp"
#include "game/item_param.hpp"
#include "game/param.hpp"
#include "game/skill_data.hpp"

namespace defender {
namespace {

const TextureHandle& game_texture(const std::string& file_name) {
    return TextureCache::instance().get("assets/imgs_480_800/game/" + file_name);
}

int item_level(const MainGameScene* scene, int type) {
    return scene->is_rep() ? ItemParam::get_level(type, false) : ItemParam::get_level(type);
}

std::string bow_texture_name(int bow_level) {
    if (bow_level <= 0) {
        return "bow_normal.png";
    }
    if (bow_level >= BowData::FINAL) {
        return "bow_final.png";
    }
    if (bow_level <= 9) {
        return "bow_pow_0" + std::to_string(bow_level) + ".png";
    }
    if (bow_level <= 18) {
        return "bow_agi_0" + std::to_string(bow_level - 9) + ".png";
    }
    return "bow_mul_0" + std::to_string(bow_level - 18) + ".png";
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

void MainGameScene::init_bow_state() {
    const int bow_level = item_level(this, ItemParam::BOW_EQUIP);
    const int agi_speed = SkillData::get_value(
        SkillData::AGI,
        BowData::get_ability(bow_level, 1)
    );
    bow_cd_ms_ = agi_speed > 0 ? (10000 / agi_speed) : 400;
    bow_cd_left_ms_ = 0;
    bow_angle_deg_ = 0.0f;
    target_angle_deg_ = 0.0f;

    Param::fatal_blow_rate = 0;
    Param::multi_power = 100;
    Param::power_shot_dis = 0;
    Param::atk_spd_dec_rate = 0;
    arrow_num_ = 1;

    if (item_level(this, ItemParam::FATAL_BLOW) > 0) {
        Param::fatal_blow_rate = SkillData::get_value(
            SkillData::FATAL_BLOW,
            BowData::get_ability(bow_level, 3)
        );
    }
    if (item_level(this, ItemParam::POISON_ARROW) > 0) {
        Param::atk_spd_dec_rate = SkillData::get_value(SkillData::POISON_ARROW);
    }
    if (item_level(this, ItemParam::POWER_SHOT) > 0) {
        Param::power_shot_dis = SkillData::get_value(
            SkillData::POWER_SHOT,
            BowData::get_ability(bow_level, 2)
        );
    }

    const int multi_level = item_level(this, ItemParam::MULTI_ARROW) +
                            BowData::get_ability(bow_level, 4);
    if (multi_level > 0) {
        Param::multi_power = SkillData::get_value(
            SkillData::MULTI_ARROW,
            BowData::get_ability(bow_level, 4)
        );
        arrow_num_ = ((multi_level - 1) / 4) + 2;
    }
    arrow_num_ = std::min(5, std::max(1, arrow_num_));

    arrow_power_ = SkillData::get_value(
        SkillData::STR,
        BowData::get_ability(bow_level, 0)
    );
    if (!is_rep()) {
        arrow_power_ = static_cast<int>(
            arrow_power_ + ((arrow_power_ * Param::extra_dmg) / 100.0f)
        );
    }
    arrow_speed_ = get_x(1000.0f);
}

void MainGameScene::draw_bow_layer() {
    const int bow_level = std::max(0, item_level(this, ItemParam::BOW_EQUIP));
    const std::string bow_name = bow_texture_name(bow_level);
    const std::string arrow_name = arrow_texture_name(bow_level);
    const auto& bow_tex = game_texture(bow_name);
    const auto& arrow_tex = game_texture(arrow_name);
    const int bow_src_w = bow_tex.valid ? game_texture_width_px(bow_tex, bow_name) : 90;
    const int bow_src_h = bow_tex.valid ? game_texture_height_px(bow_tex, bow_name) : 95;
    const float bow_draw_w = get_x(static_cast<float>(bow_src_w));
    const float bow_draw_h = get_y(static_cast<float>(bow_src_h));
    const int arrow_src_w = arrow_tex.valid ? game_texture_width_px(arrow_tex, arrow_name) : 85;
    const int arrow_src_h = arrow_tex.valid ? game_texture_height_px(arrow_tex, arrow_name) : 9;
    const float arrow_draw_w = get_x(static_cast<float>(arrow_src_w));
    const float arrow_draw_h = get_y(static_cast<float>(arrow_src_h));

    glPushMatrix();
    glTranslatef(bow_x_, bow_y_, 0.0f);
    glRotatef(bow_angle_deg_, 0.0f, 0.0f, 1.0f);

    if (bow_cd_left_ms_ <= 0) {
        if (arrow_tex.valid) {
            draw_game_texture_quad(
                arrow_tex,
                arrow_name,
                -(arrow_draw_w * 0.1f),
                -(arrow_draw_h * 0.5f),
                arrow_draw_w,
                arrow_draw_h,
                1.0f
            );
        } else {
            draw_quad(
                -(arrow_draw_w * 0.1f),
                -(arrow_draw_h * 0.5f),
                arrow_draw_w,
                arrow_draw_h,
                0.95f,
                0.92f,
                0.35f,
                1.0f
            );
        }
    }

    if (bow_tex.valid) {
        draw_game_texture_quad(
            bow_tex,
            bow_name,
            -(bow_draw_w * 0.5f),
            -(bow_draw_h * 0.5f),
            bow_draw_w,
            bow_draw_h,
            1.0f
        );
    } else {
        draw_quad(-(bow_draw_w * 0.5f), -(bow_draw_h * 0.5f), bow_draw_w, bow_draw_h, 0.62f, 0.45f, 0.18f, 1.0f);
    }
    glPopMatrix();

    if (is_shotting_) {
        draw_quad(shot_x_ - get_x(6.0f), shot_y_ - get_y(6.0f), get_x(12.0f), get_y(12.0f), 1.0f, 0.95f, 0.3f, 1.0f);
    }
}

}
