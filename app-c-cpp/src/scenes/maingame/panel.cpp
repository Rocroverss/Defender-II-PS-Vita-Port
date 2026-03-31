#include "scenes/main_game_scene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

#include "engine/abstract_game.hpp"
#include "engine/game_texture_specs.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/audio_manager.hpp"
#include "game/help_overlay.hpp"
#include "game/item_param.hpp"
#include "game/param.hpp"
#include "game/sounds.hpp"
#include "scenes/research/research_render.hpp"

namespace defender {
namespace {

struct MagicDataEntry {
    int power_base;
    int power_step;
    int range_units;
    int mana_base;
    int mana_step;
    int spe_time_base_ms;
    int spe_time_step_ms;
};

constexpr std::array<MagicDataEntry, 9> kMagicData = {{
    {50, 30, 105, 35, 0, 2000, 1000},
    {150, 50, 105, 65, 0, 3000, 1000},
    {140, 60, 105, 125, 0, 5000, 1000},
    {20, 20, 130, 35, 0, 2000, 1000},
    {35, 20, 130, 65, 0, 4000, 1000},
    {60, 40, 130, 125, 0, 7000, 1000},
    {35, 25, 120, 35, 0, 3000, 1000},
    {45, 20, 120, 65, 0, 6500, 500},
    {100, 50, 120, 125, 0, 9000, 1000},
}};
constexpr double kPi = 3.14159265358979323846;

constexpr const char* kHudNumberStrip = "assets/imgs_480_800/game/z_number_list.png";
constexpr const char* kHudNumberStripFile = "z_number_list.png";

const TextureHandle& game_texture(const std::string& file_name) {
    return TextureCache::instance().get("assets/imgs_480_800/game/" + file_name);
}

const TextureHandle& stats_texture(const std::string& file_name) {
    return TextureCache::instance().get("assets/imgs_480_800/stats/" + file_name);
}

const TextureHandle& stage_badge_texture() {
    return stats_texture(Param::language == 1 ? "small_stage_kr.png" : "small_stage.png");
}

std::string magic_button_texture_name(int magic_type) {
    switch (magic_type) {
    case 1: return "magic_button_fire.png";
    case 2: return "magic_button_fire2.png";
    case 3: return "magic_button_fire3.png";
    case 4: return "magic_button_ice.png";
    case 5: return "magic_button_ice2.png";
    case 6: return "magic_button_ice3.png";
    case 7: return "magic_button_elect.png";
    case 8: return "magic_button_elect2.png";
    case 9: return "magic_button_elect3.png";
    default: return "magic_button_lock.png";
    }
}

int item_level(const MainGameScene* scene, int type) {
    return scene->is_rep() ? ItemParam::get_level(type, false) : ItemParam::get_level(type);
}

MagicDataEntry magic_data(int magic_type) {
    if (magic_type < 1 || magic_type > static_cast<int>(kMagicData.size())) {
        return kMagicData[0];
    }
    return kMagicData[static_cast<size_t>(magic_type - 1)];
}

float hud_button_y() {
    return Scene::get_y(8.0f);
}

float hud_spell_button_y() {
    return 0.0f;
}

float hud_add_button_y() {
    return Scene::get_y(8.0f);
}

float hud_button_size_x() {
    return Scene::get_x(76.0f);
}

float hud_button_size_y() {
    return Scene::get_y(76.0f);
}

float hud_bottom_pad_x() {
    return Scene::get_x(16.0f);
}

float hud_button_gap_x() {
    return Scene::get_x(12.0f);
}

float texture_w(const TextureHandle& texture, const std::string& file_name, float scale = 1.0f) {
    if (!texture.valid) {
        return 0.0f;
    }
    return Scene::get_xy(static_cast<float>(game_texture_width_px(texture, file_name))) * scale;
}

float texture_w(const TextureHandle& texture, float scale = 1.0f) {
    if (!texture.valid) {
        return 0.0f;
    }
    return Scene::get_xy(static_cast<float>(texture.width)) * scale;
}

float texture_h(const TextureHandle& texture, const std::string& file_name, float scale = 1.0f) {
    if (!texture.valid) {
        return 0.0f;
    }
    return Scene::get_xy(static_cast<float>(game_texture_height_px(texture, file_name))) * scale;
}

float texture_h(const TextureHandle& texture, float scale = 1.0f) {
    if (!texture.valid) {
        return 0.0f;
    }
    return Scene::get_xy(static_cast<float>(texture.height)) * scale;
}

float number_strip_width(int value, float scale) {
    const auto& strip = TextureCache::instance().get(kHudNumberStrip);
    if (!strip.valid) {
        return 0.0f;
    }
    const size_t digits = std::to_string(std::max(0, value)).size();
    const float digit_w = (Scene::get_xy(static_cast<float>(
        game_texture_width_px(strip, kHudNumberStripFile))) / 10.0f) * scale;
    return digit_w * static_cast<float>(digits);
}

float number_strip_height(float scale) {
    const auto& strip = TextureCache::instance().get(kHudNumberStrip);
    if (!strip.valid) {
        return 0.0f;
    }
    return Scene::get_xy(static_cast<float>(
        game_texture_height_px(strip, kHudNumberStripFile))) * scale;
}

void draw_number_right(int value, float right_x, float y, float scale, float alpha = 1.0f) {
    draw_number_strip(kHudNumberStrip, value, right_x - number_strip_width(value, scale), y, scale, 1.0f, 1.0f, 1.0f, alpha);
}

void draw_magic_cd_pieslice(const TextureHandle& texture,
                            float center_x,
                            float center_y,
                            float scale,
                            int show_parts) {
    if (!texture.valid || texture.id == 0 || show_parts <= 0) {
        return;
    }

    const int clamped_parts = std::clamp(show_parts, 0, 300);
    const int vertex_count = clamped_parts + 2;
    std::vector<GLfloat> vertices(static_cast<size_t>(vertex_count) * 3, 0.0f);
    std::vector<GLfloat> uvs(static_cast<size_t>(vertex_count) * 2, 0.0f);

    const float logical_w = static_cast<float>(game_texture_width_px(texture, "magic_cd.png"));
    const float logical_h = static_cast<float>(game_texture_height_px(texture, "magic_cd.png"));
    const float texture_u1 = game_texture_u1(texture, "magic_cd.png");
    const float texture_v0 = game_texture_v0(texture, "magic_cd.png");
    const float tex_center_x = logical_w * 0.5f;
    const float tex_center_y = logical_h * 0.5f;
    const double one_part = (kPi * 2.0) / 300.0;

    vertices[0] = center_x;
    vertices[1] = center_y;
    uvs[0] = (tex_center_x / logical_w) * texture_u1;
    uvs[1] = texture_v0 - ((tex_center_y / logical_h) * texture_v0);

    for (int i = 0; i <= clamped_parts; ++i) {
        const double degree = (static_cast<double>(i) * one_part) + (kPi * 0.5);
        const float local_x = static_cast<float>(-std::cos(degree) * Scene::get_xy(40.0f));
        const float local_y = static_cast<float>(std::sin(degree) * Scene::get_xy(40.0f));
        const int vertex_index = i + 1;
        vertices[static_cast<size_t>(vertex_index) * 3] = center_x + (local_x * scale);
        vertices[(static_cast<size_t>(vertex_index) * 3) + 1] = center_y + (local_y * scale);

        const float tex_x = tex_center_x + static_cast<float>(-std::cos(degree) * 40.0);
        const float tex_y = tex_center_y + static_cast<float>(std::sin(degree) * 40.0);
        uvs[static_cast<size_t>(vertex_index) * 2] = (tex_x / logical_w) * texture_u1;
        uvs[(static_cast<size_t>(vertex_index) * 2) + 1] = texture_v0 - ((tex_y / logical_h) * texture_v0);
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glTexCoordPointer(2, GL_FLOAT, 0, uvs.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

}

void MainGameScene::init_magic_state() {
    magic_slots_.fill(MagicSlot{});
    selected_magic_slot_ = -1;
    is_spelling_ = false;
    magic_show_range_ = false;

    for (int slot = 0; slot < 3; ++slot) {
        const int equip_level = item_level(this, ItemParam::MAGIC1_EQUIP + slot);
        if (equip_level < 0) {
            continue;
        }
        const int magic_type = (slot * 3) + 1 + equip_level;
        const int research_level = item_level(this, ItemParam::FIRE_1 + (slot * 3) + equip_level);
        const MagicDataEntry entry = magic_data(magic_type);
        auto& magic_slot = magic_slots_[static_cast<size_t>(slot)];
        magic_slot.magic_type = magic_type;
        magic_slot.power = entry.power_base + (entry.power_step * research_level);
        magic_slot.spe_time = (entry.spe_time_base_ms + (entry.spe_time_step_ms * research_level)) / 1000;
        magic_slot.mana_cost = entry.mana_base + (entry.mana_step * research_level);
        magic_slot.range_units = entry.range_units;
        magic_slot.total_cd_ms = 10000;
        magic_slot.cd_left_ms = 0;
    }
}

bool MainGameScene::panel_touch(const TouchEvent& event) {
    const float btn_w = hud_button_size_x();
    const float btn_h = hud_button_size_y();
    const float add_btn_y = hud_add_button_y();
    const float spell_btn_y = hud_spell_button_y();
    const float add_btn_x0 = hud_bottom_pad_x();
    const float add_btn_x1 = add_btn_x0 + btn_w;
    const float add_btn_y0 = add_btn_y;
    const float add_btn_y1 = add_btn_y + btn_h;
    const float spell_y0 = spell_btn_y;
    const float spell_y1 = spell_btn_y + btn_h;
    const std::array<float, 3> spell_x0 = {
        Scene::screen_width - hud_bottom_pad_x() - (btn_w * 3.0f) - (hud_button_gap_x() * 2.0f),
        Scene::screen_width - hud_bottom_pad_x() - (btn_w * 2.0f) - hud_button_gap_x(),
        Scene::screen_width - hud_bottom_pad_x() - btn_w
    };
    const std::array<float, 3> spell_x1 = {
        spell_x0[0] + btn_w,
        spell_x0[1] + btn_w,
        spell_x0[2] + btn_w
    };

    if (is_spelling_) {
        if (event.action == TouchAction::Move) {
            magic_update_range(event.x1, event.y1);
            return true;
        }
        if (event.action == TouchAction::Up) {
            const int slot = selected_magic_slot_;
            is_spelling_ = false;
            selected_magic_slot_ = -1;
            magic_cancel();
            if (slot == 0) {
                cast_spell_fire(event.x1, event.y1);
            } else if (slot == 1) {
                cast_spell_ice(event.x1, event.y1);
            } else if (slot == 2) {
                cast_spell_lightning(event.x1, event.y1);
            }
            return true;
        }
    }

    if (event.action != TouchAction::Down) {
        return false;
    }

    if (event.x1 >= add_btn_x0 && event.x1 <= add_btn_x1 &&
        event.y1 >= add_btn_y0 && event.y1 <= add_btn_y1) {
        if (Param::stone > 0 && wall_mana_ < wall_max_mana_) {
            --Param::stone;
            ++Param::cost_stone;
            ++Param::add_mana_data;
            wall_mana_ += static_cast<int>(wall_max_mana_ * 0.4f);
            wall_mana_ += (wall_max_mana_ * Param::extra_mana) / 100;
            wall_mana_ = std::min(wall_max_mana_, wall_mana_);
            AudioManager::instance().play_sound(Sounds::MANA_REC);
        }
        return true;
    }

    for (int slot = 0; slot < 3; ++slot) {
        const auto& magic_slot = magic_slots_[static_cast<size_t>(slot)];
        if (event.x1 >= spell_x0[static_cast<size_t>(slot)] &&
            event.x1 <= spell_x1[static_cast<size_t>(slot)] &&
            event.y1 >= spell_y0 &&
            event.y1 <= spell_y1 &&
            magic_slot.magic_type != 0 &&
            magic_slot.cd_left_ms <= 0 &&
            wall_mana_ >= magic_slot.mana_cost) {
            selected_magic_slot_ = slot;
            is_spelling_ = true;
            magic_show_range(event.x1, event.y1, magic_slot.range_units);
            return true;
        }
    }

    return false;
}

void MainGameScene::draw_panel_layer(float hp_ratio, float mana_ratio) {
    const float btn_w = hud_button_size_x();
    const float btn_h = hud_button_size_y();
    const float add_btn_y = hud_add_button_y();
    const float spell_btn_y = hud_spell_button_y();
    const float add_btn_x = hud_bottom_pad_x();
    const float hp_ratio_clamped = std::clamp(hp_ratio, 0.0f, 1.0f);
    const float mana_ratio_clamped = std::clamp(mana_ratio, 0.0f, 1.0f);

    float wave_ratio = 0.0f;
    if (is_game_finish_) {
        wave_ratio = 1.0f;
    } else if (boss_spawned_ && !boss_killed_) {
        wave_ratio = 0.95f;
    } else if (stage_total_time_sec_ > 0) {
        const float total_ms = static_cast<float>(stage_total_time_sec_ * 1000);
        wave_ratio = std::clamp(static_cast<float>(AbstractGame::game_time_ms()) / total_ms, 0.0f, 1.0f);
    }

    const auto& coin_tex = game_texture("coin.png");
    const auto& stone_tex = game_texture("mana_stone.png");
    const auto& stage_tex = stage_badge_texture();
    const auto& sword_tex = game_texture("sword_logo.png");
    const auto& monster_tex = game_texture("monster_logo.png");
    const auto& panel_tex = game_texture("blood_panel.png");
    const auto& panel_bg_tex = game_texture("z_blood_panel_bg.png");
    const auto& panel_hp_tex = game_texture("blood_panel_red.png");
    const auto& panel_mana_tex = game_texture("blood_panel_blue.png");

    const float coin_w = texture_w(coin_tex, "coin.png");
    const float coin_h = texture_h(coin_tex, "coin.png");
    const float coin_x = get_x(130.0f);
    const float coin_y = get_y(440.0f);
    const float gold_scale = 0.60f;
    const float gold_h = number_strip_height(gold_scale);
    if (coin_tex.valid) {
        draw_game_texture_quad(coin_tex, "coin.png", coin_x, coin_y, coin_w, coin_h, 1.0f);
    }
    draw_number_strip(
        kHudNumberStrip,
        std::max(0, Param::gold),
        coin_x + (coin_w * 1.5f),
        coin_y + std::max(0.0f, (coin_h - gold_h) * 0.5f),
        gold_scale,
        1.0f,
        1.0f,
        1.0f,
        1.0f
    );

    const float stone_scale = 0.90f;
    const float stone_w = texture_w(stone_tex, "mana_stone.png", stone_scale);
    const float stone_h = texture_h(stone_tex, "mana_stone.png", stone_scale);
    const float stone_x = get_x(280.0f);
    const float stone_y = get_y(440.0f);
    const float stone_num_scale = 0.68f;
    const float stone_num_h = number_strip_height(stone_num_scale);
    if (stone_tex.valid) {
        draw_game_texture_quad(stone_tex, "mana_stone.png", stone_x, stone_y, stone_w, stone_h, 1.0f);
    }
    draw_number_strip(
        kHudNumberStrip,
        std::max(0, Param::stone),
        stone_x + (stone_w * 1.4f),
        stone_y + std::max(0.0f, (stone_h - stone_num_h) * 0.5f),
        stone_num_scale,
        1.0f,
        1.0f,
        1.0f,
        1.0f
    );

    const float stage_w = texture_w(stage_tex);
    const float stage_h = texture_h(stage_tex);
    const float stage_x = get_x(385.0f);
    const float stage_y = get_y(437.0f);
    const float stage_num_scale = 0.60f;
    const float stage_num_h = number_strip_height(stage_num_scale);
    if (stage_tex.valid) {
        draw_textured_quad(stage_tex.id, stage_x, stage_y, stage_w, stage_h, 1.0f);
    }
    draw_number_strip(
        kHudNumberStrip,
        std::max(1, Param::stage),
        stage_x + (stage_w * 1.05f),
        stage_y + std::max(0.0f, (stage_h - stage_num_h) * 0.5f),
        stage_num_scale,
        1.0f,
        1.0f,
        1.0f,
        1.0f
    );

    const float sword_w = sword_tex.valid ? texture_w(sword_tex, "sword_logo.png") : get_x(232.0f);
    const float sword_h = sword_tex.valid ? texture_h(sword_tex, "sword_logo.png") : get_y(36.0f);
    const float monster_w = monster_tex.valid ? texture_w(monster_tex, "monster_logo.png") : get_x(34.0f);
    const float monster_h = monster_tex.valid ? texture_h(monster_tex, "monster_logo.png") : get_y(34.0f);
    const float sword_x = Scene::screen_width - (sword_w * 1.05f);
    const float sword_y = get_y(480.0f) - (sword_h * 1.20f);
    const float progress_phase = std::clamp((1.0f - wave_ratio) * 1000.0f, 0.0f, 1000.0f);
    const float track_x = sword_x - (sword_w * 0.03f);
    const float track_w = sword_w * 0.0008f;
    const float monster_x = track_x + (track_w * progress_phase);
    const float monster_y = sword_y - (sword_h * 0.10f);

    if (sword_tex.valid) {
        draw_game_texture_quad(sword_tex, "sword_logo.png", sword_x, sword_y, sword_w, sword_h, 0.98f);
    } else {
        draw_quad(sword_x, sword_y + get_y(10.0f), sword_w, get_y(10.0f), 0.14f, 0.14f, 0.16f, 0.85f);
        draw_quad(sword_x, sword_y + get_y(10.0f), sword_w * wave_ratio, get_y(10.0f), 0.98f, 0.82f, 0.22f, 0.95f);
    }

    if (monster_tex.valid) {
        draw_game_texture_quad(monster_tex, "monster_logo.png", monster_x, monster_y, monster_w, monster_h, 1.0f);
    } else {
        draw_quad(monster_x, monster_y, monster_w, monster_h, 0.85f, 0.18f, 0.18f, 0.95f);
    }

    const float panel_scale = 1.15f;
    const float panel_w = panel_tex.valid ? texture_w(panel_tex, "blood_panel.png", panel_scale) : get_x(220.0f);
    const float panel_h = panel_tex.valid ? texture_h(panel_tex, "blood_panel.png", panel_scale) : get_y(72.0f);
    const float panel_x = get_x(3.0f);
    const float panel_y = get_y(0.0f);
    const float mana_bar_x = panel_x + (panel_w * 0.24f);
    const float mana_bar_y = panel_y + (panel_h * 0.22f);
    const float hp_bar_x = panel_x + (panel_w * 0.26f);
    const float hp_bar_y = panel_y + (panel_h * 0.40f);
    const int mana_draw_percent = std::clamp(static_cast<int>(mana_ratio_clamped * 1000.0f), 0, 1000);
    const int hp_draw_percent = std::clamp(static_cast<int>(hp_ratio_clamped * 1000.0f), 0, 1000);
    const float mana_bar_bg_w = panel_bg_tex.valid
        ? texture_w(panel_bg_tex, "z_blood_panel_bg.png") * 160.0f * panel_scale
        : panel_w * 0.38f;
    const float mana_bar_bg_h = panel_bg_tex.valid
        ? texture_h(panel_bg_tex, "z_blood_panel_bg.png") * panel_scale
        : panel_h * 0.15f;
    const float hp_bar_bg_w = panel_bg_tex.valid
        ? texture_w(panel_bg_tex, "z_blood_panel_bg.png") * 175.0f * panel_scale
        : panel_w * 0.42f;
    const float hp_bar_bg_h = panel_bg_tex.valid
        ? texture_h(panel_bg_tex, "z_blood_panel_bg.png") * panel_scale
        : panel_h * 0.16f;
    const float mana_piece_w = panel_mana_tex.valid
        ? texture_w(panel_mana_tex, "blood_panel_blue.png") * (0.39f + (0.04f * mana_draw_percent)) * panel_scale
        : mana_bar_bg_w * mana_ratio_clamped;
    const float mana_piece_h = panel_mana_tex.valid
        ? texture_h(panel_mana_tex, "blood_panel_blue.png") * panel_scale
        : mana_bar_bg_h;
    const float hp_piece_w = panel_hp_tex.valid
        ? texture_w(panel_hp_tex, "blood_panel_red.png") * (0.4f + (0.043f * hp_draw_percent)) * panel_scale
        : hp_bar_bg_w * hp_ratio_clamped;
    const float hp_piece_h = panel_hp_tex.valid
        ? texture_h(panel_hp_tex, "blood_panel_red.png") * panel_scale
        : hp_bar_bg_h;

    if (panel_bg_tex.valid) {
        draw_game_texture_quad(
            panel_bg_tex,
            "z_blood_panel_bg.png",
            mana_bar_x,
            mana_bar_y,
            mana_bar_bg_w,
            mana_bar_bg_h,
            0.95f
        );
        const int hp_flash_phase = static_cast<int>(AbstractGame::game_time_ms() % 700ULL);
        const float hp_bg_tint = hp_ratio_clamped < 0.30f
            ? (std::abs(350 - hp_flash_phase) / 500.0f) + 0.3f
            : 1.0f;
        draw_game_texture_quad_tinted(
            panel_bg_tex,
            "z_blood_panel_bg.png",
            hp_bar_x,
            hp_bar_y,
            hp_bar_bg_w,
            hp_bar_bg_h,
            hp_bg_tint,
            hp_bg_tint,
            hp_bg_tint,
            1.0f
        );
    } else {
        draw_quad(mana_bar_x, mana_bar_y, mana_bar_bg_w, mana_bar_bg_h, 0.07f, 0.09f, 0.16f, 0.90f);
        draw_quad(hp_bar_x, hp_bar_y, hp_bar_bg_w, hp_bar_bg_h, 0.12f, 0.08f, 0.08f, 0.90f);
    }

    if (panel_mana_tex.valid) {
        draw_game_texture_quad(
            panel_mana_tex,
            "blood_panel_blue.png",
            mana_bar_x,
            mana_bar_y,
            mana_piece_w,
            mana_piece_h,
            1.0f
        );
    } else if (mana_ratio_clamped > 0.0f) {
        draw_quad(mana_bar_x, mana_bar_y, mana_piece_w, mana_piece_h, 0.20f, 0.55f, 1.0f, 0.95f);
    }

    if (panel_hp_tex.valid) {
        draw_game_texture_quad(
            panel_hp_tex,
            "blood_panel_red.png",
            hp_bar_x,
            hp_bar_y,
            hp_piece_w,
            hp_piece_h,
            1.0f
        );
    } else if (hp_ratio_clamped > 0.0f) {
        draw_quad(hp_bar_x, hp_bar_y, hp_piece_w, hp_piece_h, 0.90f, 0.2f, 0.2f, 0.95f);
    }

    if (panel_tex.valid) {
        draw_game_texture_quad(panel_tex, "blood_panel.png", panel_x, panel_y, panel_w, panel_h, 1.0f);
    } else {
        draw_quad(panel_x, panel_y, panel_w, panel_h, 0.16f, 0.10f, 0.10f, 0.95f);
    }

    const float hp_number_scale = 0.50f;
    const float mana_number_scale = 0.45f;
    const float hp_number_h = number_strip_height(hp_number_scale);
    const float mana_number_h = number_strip_height(mana_number_scale);
    draw_number_right(
        std::max(0, wall_hp_),
        get_x(160.0f),
        hp_bar_y + ((hp_piece_h - hp_number_h) * 0.5f),
        hp_number_scale
    );
    draw_number_right(
        std::max(0, wall_mana_),
        get_x(140.0f),
        mana_bar_y + ((mana_piece_h - mana_number_h) * 0.5f),
        mana_number_scale
    );

    const auto& add_mana_tex = game_texture("button_addmana.png");
    const bool can_add_mana = Param::stone > 0 && wall_mana_ < wall_max_mana_;
    const int mana_flash_phase = static_cast<int>(AbstractGame::game_time_ms() % 1000ULL);
    const float add_pulse = can_add_mana
        ? 0.5f + (std::abs(500 - mana_flash_phase) / 1000.0f)
        : 0.45f;
    if (add_mana_tex.valid) {
        draw_game_texture_quad_tinted(
            add_mana_tex,
            "button_addmana.png",
            add_btn_x,
            add_btn_y,
            btn_w,
            btn_h,
            add_pulse,
            add_pulse,
            add_pulse,
            can_add_mana ? 1.0f : 0.45f
        );
    } else {
        draw_quad(add_btn_x, add_btn_y, btn_w, btn_h, 0.25f, 0.78f, 0.24f, can_add_mana ? 0.95f : 0.45f);
    }

    const std::array<float, 3> btn_x = {
        Scene::screen_width - hud_bottom_pad_x() - (btn_w * 3.0f) - (hud_button_gap_x() * 2.0f),
        Scene::screen_width - hud_bottom_pad_x() - (btn_w * 2.0f) - hud_button_gap_x(),
        Scene::screen_width - hud_bottom_pad_x() - btn_w
    };
    const auto& lock_btn = game_texture("magic_button_lock.png");
    const auto& lowmana_bg = game_texture("magic_button_lowmana_bg.png");
    const auto& lowmana_word = game_texture("magic_button_lowmana_word.png");
    const auto& magic_cd = game_texture("magic_cd.png");

    for (int slot = 0; slot < 3; ++slot) {
        const auto& magic_slot = magic_slots_[static_cast<size_t>(slot)];
        const bool unlocked = magic_slot.magic_type != 0;
        const bool low_mana = unlocked && wall_mana_ < magic_slot.mana_cost;
        const bool on_cd = unlocked && magic_slot.cd_left_ms > 0;
        const bool ready = unlocked && !low_mana && !on_cd;
        const std::string button_name = magic_button_texture_name(magic_slot.magic_type);
        const auto& button_tex = game_texture(button_name);
        if (!unlocked) {
            if (lock_btn.valid) {
                draw_game_texture_quad(
                    lock_btn,
                    "magic_button_lock.png",
                    btn_x[static_cast<size_t>(slot)],
                    spell_btn_y,
                    btn_w,
                    btn_h,
                    1.0f
                );
            } else {
                draw_quad(btn_x[static_cast<size_t>(slot)], spell_btn_y, btn_w, btn_h, 0.16f, 0.16f, 0.18f, 0.95f);
            }
            continue;
        }

        if (button_tex.valid) {
            draw_game_texture_quad(
                button_tex,
                button_name,
                btn_x[static_cast<size_t>(slot)],
                spell_btn_y,
                btn_w,
                btn_h,
                ready ? 1.0f : 0.9f
            );
        } else {
            const float r = slot == 0 ? 0.95f : (slot == 1 ? 0.2f : 0.95f);
            const float g = slot == 0 ? 0.25f : (slot == 1 ? 0.75f : 0.95f);
            const float b = slot == 0 ? 0.15f : (slot == 1 ? 1.0f : 0.25f);
            draw_quad(btn_x[static_cast<size_t>(slot)], spell_btn_y, btn_w, btn_h, r, g, b, ready ? 0.95f : 0.85f);
        }

        if (on_cd && lock_btn.valid) {
            draw_game_texture_quad(
                lock_btn,
                "magic_button_lock.png",
                btn_x[static_cast<size_t>(slot)],
                spell_btn_y,
                btn_w,
                btn_h,
                0.9f
            );
        }

        if (low_mana) {
            if (lowmana_bg.valid) {
                draw_game_texture_quad(
                    lowmana_bg,
                    "magic_button_lowmana_bg.png",
                    btn_x[static_cast<size_t>(slot)],
                    spell_btn_y,
                    btn_w,
                    btn_h,
                    1.0f
                );
            }
            if (lowmana_word.valid) {
                const float pulse = std::abs(500.0f - static_cast<float>(AbstractGame::game_time_ms() % 1000ULL)) / 500.0f;
                draw_game_texture_quad_tinted(
                    lowmana_word,
                    "magic_button_lowmana_word.png",
                    btn_x[static_cast<size_t>(slot)],
                    spell_btn_y,
                    btn_w,
                    btn_h,
                    1.0f,
                    pulse,
                    pulse,
                    1.0f
                );
            }
        }

        if (on_cd && magic_cd.valid) {
            const int progress = ((magic_slot.total_cd_ms - magic_slot.cd_left_ms) * 1000) /
                std::max(1, magic_slot.total_cd_ms);
            draw_magic_cd_pieslice(
                magic_cd,
                btn_x[static_cast<size_t>(slot)] + (btn_w * 0.5f),
                spell_btn_y + (btn_h * 0.5f),
                1.0f,
                (progress * 300) / 1000
            );
        }
    }

    HelpOverlay::draw();
}

} // namespace defender
