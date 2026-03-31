#include "game/help_overlay.hpp"

#include <array>
#include <cstdint>
#include <string>

#include "engine/abstract_game.hpp"
#include "engine/font_renderer.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "scenes/scene.hpp"

namespace defender {
namespace {

struct HelpState {
    int type = 0;
    int64_t time_flag_ms = 0;
    float alpha = 0.0f;
    bool is_show = false;
    bool active_flag = false;
    std::array<bool, 30> runtime_flag = {};
};

HelpState s_state;

constexpr const char* kHelpFrame = "assets/imgs_480_800/needed/help_frame.png";
constexpr const char* kHelpBattleFrame = "assets/imgs_480_800/needed/help_battle_frame.png";
constexpr const char* kHelpMan = "assets/imgs_480_800/needed/help_man.png";
constexpr const char* kHelpHand = "assets/imgs_480_800/needed/help_hand.png";
constexpr const char* kHelpMana = "assets/imgs_480_800/needed/zz_mana_1.png";

constexpr float kEquipMagic1Left = 205.0f;
constexpr float kEquipMagic1Top = 404.0f;
constexpr float kEquipMagic1Right = 260.0f;
constexpr float kEquipMagic1Bottom = 460.0f;
constexpr float kEquipMagic2Left = 270.0f;
constexpr float kEquipMagic2Top = 404.0f;
constexpr float kEquipMagic2Right = 325.0f;
constexpr float kEquipMagic2Bottom = 460.0f;

constexpr float kHudManaLeft = 16.0f;
constexpr float kHudManaTop = 392.0f;
constexpr float kHudManaRight = 92.0f;
constexpr float kHudManaBottom = 468.0f;
constexpr float kHudSpell1Left = 532.0f;
constexpr float kHudSpell1Top = 392.0f;
constexpr float kHudSpell1Right = 608.0f;
constexpr float kHudSpell1Bottom = 468.0f;

bool contains_box(float left, float top, float right, float bottom, float x, float y) {
    return x >= Scene::get_x(left) && x <= Scene::get_x(right) &&
           y >= Scene::get_y(top) && y <= Scene::get_y(bottom);
}

std::string save_key(int type) {
    return std::string(Save::HELP) + std::to_string(type);
}

const char* bg_for_type(int type) {
    switch (type) {
    case 1:
    case 11:
        return "assets/imgs_480_800/needed/help_0_black.png";
    case 2:
        return "assets/imgs_480_800/needed/help_1_name.png";
    case 3:
        return "assets/imgs_480_800/needed/help_2_level.png";
    case 4:
        return "assets/imgs_480_800/needed/help_3_achieve.png";
    case 5:
        return "assets/imgs_480_800/needed/help_4_infomation.png";
    case 6:
        return "assets/imgs_480_800/needed/help_5_start.png";
    case 7:
    case 8:
        return "assets/imgs_480_800/needed/help_8_unlock.png";
    case 9:
    case 14:
        return "assets/imgs_480_800/needed/help_9_magic.png";
    case 10:
    case 15:
        return "assets/imgs_480_800/needed/help_10_magic2.png";
    case 12:
        return "assets/imgs_480_800/needed/help_6_cast.png";
    case 13:
        return "assets/imgs_480_800/needed/help_7_recover.png";
    default:
        return "assets/imgs_480_800/needed/help_0_black.png";
    }
}

const char* text_for_type(int type) {
    switch (type) {
    case 1:
        return "Welcome to Defender II.";
    case 2:
        return "Tap your name plate to edit your profile.";
    case 3:
        return "This bar shows your level and experience.";
    case 4:
        return "Achievements unlock permanent bonus rewards.";
    case 5:
        return "Check your stage and battle information here.";
    case 6:
        return "Tap CONTINUE to begin the fight.";
    case 7:
        return "Battle mode is now unlocked.";
    case 8:
        return "Upgrade before challenging battle mode.";
    case 9:
        return "Equip your first magic spell.";
    case 10:
        return "Equip your second magic spell.";
    case 11:
        return "Hold and drag to aim your bow.";
    case 12:
        return "Drag from a magic button to cast a spell.";
    case 13:
        return "Tap the mana button to recover mana.";
    case 14:
        return "Tap the first magic slot.";
    case 15:
        return "Tap the second magic slot.";
    default:
        return "";
    }
}

void draw_sprite_full(const char* path, float alpha) {
    const auto& tex = TextureCache::instance().get(path);
    if (tex.valid) {
        draw_textured_quad(tex.id, 0.0f, 0.0f, Scene::screen_width, Scene::screen_height, alpha);
    } else {
        draw_quad(0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 0.0f, 0.0f, 0.0f, alpha);
    }
}

void draw_centered(const char* path, float cx, float cy, float fallback_w, float fallback_h, float alpha) {
    const auto& tex = TextureCache::instance().get(path);
    const float w = Scene::get_xy(fallback_w > 0.0f ? fallback_w : static_cast<float>(tex.width));
    const float h = Scene::get_xy(fallback_h > 0.0f ? fallback_h : static_cast<float>(tex.height));
    const float x = cx - (w * 0.5f);
    const float y = cy - (h * 0.5f);
    if (tex.valid) {
        draw_textured_quad(tex.id, x, y, w, h, alpha);
    } else {
        draw_quad(x, y, w, h, 0.16f, 0.16f, 0.16f, alpha);
    }
}

void draw_highlight_box(float left, float top, float right, float bottom, float alpha) {
    const float x = Scene::get_x(left);
    const float y = Scene::get_y(top);
    const float w = Scene::get_x(right - left);
    const float h = Scene::get_y(bottom - top);
    const float edge = Scene::get_xy(4.0f);
    draw_quad(x, y, w, h, 1.0f, 0.92f, 0.35f, 0.18f * alpha);
    draw_quad(x, y, w, edge, 1.0f, 0.92f, 0.35f, 0.75f * alpha);
    draw_quad(x, y + h - edge, w, edge, 1.0f, 0.92f, 0.35f, 0.75f * alpha);
    draw_quad(x, y, edge, h, 1.0f, 0.92f, 0.35f, 0.75f * alpha);
    draw_quad(x + w - edge, y, edge, h, 1.0f, 0.92f, 0.35f, 0.75f * alpha);
}

void draw_hand_hint(float cx, float cy, float alpha) {
    draw_centered(kHelpHand, Scene::get_x(cx), Scene::get_y(cy), 78.0f, 78.0f, alpha);
}

void set_runtime_help(int type) {
    s_state.type = type;
    s_state.time_flag_ms = 0;
    s_state.alpha = 0.0f;
    s_state.active_flag = false;
    s_state.is_show = true;
}

void close_help() {
    s_state.is_show = false;
}

void save_sequence_complete(int start_type) {
    Save::save_data(save_key(start_type), 1);
}

bool handle_touch_progress(float x, float y) {
    switch (s_state.type) {
    case 1:
        s_state.type = 2;
        return true;
    case 2:
        if (s_state.active_flag) {
            s_state.type = 3;
        }
        return true;
    case 3:
        s_state.type = 4;
        return true;
    case 4:
        s_state.type = 5;
        return true;
    case 5:
        s_state.type = 6;
        save_sequence_complete(1);
        return true;
    case 6:
        close_help();
        return false;
    case 7:
        s_state.type = 8;
        return true;
    case 8:
        s_state.type = 9;
        return true;
    case 9:
        s_state.type = 10;
        return true;
    case 10:
        save_sequence_complete(7);
        close_help();
        return false;
    case 11:
    case 12:
    case 13:
        Save::save_data(save_key(s_state.type), 1);
        AbstractGame::reset_game_time(static_cast<uint64_t>(s_state.time_flag_ms));
        close_help();
        return false;
    case 14:
        if (contains_box(kEquipMagic1Left, kEquipMagic1Top, kEquipMagic1Right, kEquipMagic1Bottom, x, y)) {
            s_state.type = 15;
            return false;
        }
        return true;
    case 15:
        if (contains_box(kEquipMagic2Left, kEquipMagic2Top, kEquipMagic2Right, kEquipMagic2Bottom, x, y)) {
            Save::save_data(save_key(14), 1);
            close_help();
            return false;
        }
        return true;
    default:
        close_help();
        return false;
    }
}

} // namespace

void HelpOverlay::init() {
    s_state = HelpState{};
}

void HelpOverlay::reset_runtime() {
    s_state.type = 0;
    s_state.time_flag_ms = 0;
    s_state.alpha = 0.0f;
    s_state.is_show = false;
    s_state.active_flag = false;
}

bool HelpOverlay::is_show() {
    return s_state.is_show || s_state.alpha > 0.0f;
}

void HelpOverlay::active() {
    if (!s_state.is_show) {
        return;
    }
    s_state.active_flag = true;
    if (s_state.type == 2) {
        s_state.type = 3;
    }
}

void HelpOverlay::set_help(int type) {
    if (type <= 0 || type >= static_cast<int>(s_state.runtime_flag.size())) {
        return;
    }
    if (Save::load_data(save_key(type)) != 0) {
        return;
    }
    set_runtime_help(type);
}

bool HelpOverlay::touch(const TouchEvent& event, float x, float y) {
    if (!is_show()) {
        return false;
    }
    if (event.action == TouchAction::Down && s_state.alpha >= 1.0f) {
        return handle_touch_progress(x, y);
    }
    return true;
}

void HelpOverlay::draw() {
    if (!is_show() || s_state.type == 0) {
        return;
    }

    const float alpha = s_state.alpha;
    draw_sprite_full(bg_for_type(s_state.type), alpha);

    const bool battle_frame = s_state.type >= 11 && s_state.type <= 13;
    draw_centered(
        battle_frame ? kHelpBattleFrame : kHelpFrame,
        Scene::get_x(400.0f),
        Scene::get_y(240.0f),
        620.0f,
        battle_frame ? 220.0f : 260.0f,
        alpha
    );

    const auto& man = TextureCache::instance().get(kHelpMan);
    const float man_w = Scene::get_xy(130.0f);
    const float man_h = Scene::get_xy(210.0f);
    if (man.valid) {
        draw_textured_quad(man.id, Scene::screen_width - man_w, 0.0f, man_w, man_h, alpha);
    }

    auto& fonts = FontRenderer::instance();
    if (!battle_frame) {
        fonts.draw_text(
            FontFaceId::Cooper,
            "HELP",
            Scene::get_x(300.0f),
            Scene::get_y(300.0f),
            Scene::get_y(28.0f),
            1.0f,
            0.95f,
            0.75f,
            alpha
        );
        fonts.draw_text(
            FontFaceId::Ants,
            text_for_type(s_state.type),
            Scene::get_x(185.0f),
            Scene::get_y(255.0f),
            Scene::get_y(20.0f),
            0.95f,
            0.98f,
            1.0f,
            alpha
        );
        if (s_state.type == 14) {
            draw_highlight_box(kEquipMagic1Left, kEquipMagic1Top, kEquipMagic1Right, kEquipMagic1Bottom, alpha);
            draw_hand_hint(250.0f, 378.0f, alpha);
        } else if (s_state.type == 15) {
            draw_highlight_box(kEquipMagic2Left, kEquipMagic2Top, kEquipMagic2Right, kEquipMagic2Bottom, alpha);
            draw_hand_hint(315.0f, 378.0f, alpha);
        }
        return;
    }

    fonts.draw_text(
        FontFaceId::Ants,
        text_for_type(s_state.type),
        Scene::get_x(190.0f),
        Scene::get_y(232.0f),
        Scene::get_y(20.0f),
        0.95f,
        0.98f,
        1.0f,
        alpha
    );
    if (s_state.type == 12) {
        draw_highlight_box(kHudSpell1Left, kHudSpell1Top, kHudSpell1Right, kHudSpell1Bottom, alpha);
        draw_hand_hint(602.0f, 366.0f, alpha);
    } else if (s_state.type == 13) {
        draw_highlight_box(kHudManaLeft, kHudManaTop, kHudManaRight, kHudManaBottom, alpha);
        draw_centered(kHelpMana, Scene::get_x(54.0f), Scene::get_y(430.0f), 72.0f, 72.0f, alpha);
        draw_hand_hint(96.0f, 364.0f, alpha);
    }
}

void HelpOverlay::update(bool allow_battle_script) {
    if (allow_battle_script && !Param::is_online_mode && Param::stage == 1) {
        const int64_t game_time_ms = static_cast<int64_t>(AbstractGame::game_time_ms());
        if (game_time_ms > 2000 && !s_state.runtime_flag[11]) {
            set_help(11);
            s_state.time_flag_ms = game_time_ms;
            s_state.runtime_flag[11] = true;
        }
        if (game_time_ms > 10100 && !s_state.runtime_flag[12] && !s_state.is_show) {
            set_help(12);
            s_state.time_flag_ms = game_time_ms;
            s_state.runtime_flag[12] = true;
        }
        if (game_time_ms > 15000 && !s_state.runtime_flag[13] && !s_state.is_show) {
            set_help(13);
            s_state.time_flag_ms = game_time_ms;
            s_state.runtime_flag[13] = true;
        }
    }

    const float delta = static_cast<float>(AbstractGame::last_span_ms()) / 250.0f;
    if (s_state.is_show) {
        s_state.alpha += delta;
        if (s_state.alpha > 1.0f) {
            s_state.alpha = 1.0f;
        }
    } else if (s_state.alpha > 0.0f) {
        s_state.alpha -= delta;
        if (s_state.alpha < 0.0f) {
            s_state.alpha = 0.0f;
            s_state.type = 0;
        }
    }
}

} // namespace defender
