#include "scenes/online_data_scene.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "engine/abstract_game.hpp"
#include "engine/font_renderer.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/achv_data.hpp"
#include "game/audio_manager.hpp"
#include "game/help_overlay.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "game/sounds.hpp"
#include "game/xp_data.hpp"

namespace defender {

namespace {

enum class TextAlign {
    Left,
    Center,
    Right
};

struct LayoutTweak {
    float x;
    float y;
    float scale;
};

enum class OnlineDataLayoutId : int {
    PlayerName = 0,
    Level,
    XpNumbers,
    LevelProgress,
    Stage,
    Count
};

constexpr const char* kBg = "assets/imgs_480_800/onlinedata/z_online_data_bg.jpg";
constexpr const char* kBgKr = "assets/imgs_480_800/onlinedata/z_online_data_bg_kr.jpg";

constexpr const char* kSingleUp = "assets/imgs_480_800/onlinedata/z_button_start_up.png";
constexpr const char* kSingleDown = "assets/imgs_480_800/onlinedata/z_button_start_down.png";
constexpr const char* kBattleUp = "assets/imgs_480_800/onlinedata/button_start_up.png";
constexpr const char* kBattleDown = "assets/imgs_480_800/onlinedata/button_start_down.png";
constexpr const char* kBattleUpKr = "assets/imgs_480_800/onlinedata/button_start_up_kr.png";
constexpr const char* kBattleDownKr = "assets/imgs_480_800/onlinedata/button_start_down_kr.png";
constexpr const char* kOffFlag = "assets/imgs_480_800/shop/off_flag.png";

constexpr const char* kExPanel = "assets/imgs_480_800/onlinedata/ex_panel.png";
constexpr const char* kExPanelBg = "assets/imgs_480_800/onlinedata/ex_piece_bg.png";
constexpr const char* kExPiece = "assets/imgs_480_800/onlinedata/ex_piece.png";

constexpr const char* kAchvLogoBg = "assets/imgs_480_800/onlinedata/achieve_bg_panel.png";
constexpr const char* kAchvPanelBg = "assets/imgs_480_800/achieve/achieve_panel_bg.png";
constexpr const char* kAchvPieceBg = "assets/imgs_480_800/achieve/achieve_piece_bg.png";
constexpr const char* kAchvPieceFrame = "assets/imgs_480_800/achieve/achieve_piece_frame.png";
constexpr const char* kAchvPiece = "assets/imgs_480_800/achieve/achieve_piece.png";
constexpr const char* kAchvCast0 = "assets/imgs_480_800/achieve/achieve_logo_cast_0.png";

constexpr float kSingleX = 153.0f;
constexpr float kSingleY = 100.0f;
constexpr float kBattleX = 485.0f;
constexpr float kBattleY = 100.0f;

constexpr float kPanelX = 438.0f;
constexpr float kPanelY = 389.0f;
constexpr float kPieceX = 442.0f;
constexpr float kPieceY = 393.0f;
constexpr float kPieceW = 4.0f;
constexpr float kPieceH = 15.0f;
constexpr float kPieceScaleX = 62.0f;

constexpr float kNameBoxX = 130.0f;
constexpr float kNameBoxY = 385.0f;
constexpr float kNameBoxW = 185.0f;
constexpr float kNameBoxH = 35.0f;

constexpr float kLevelBoxX = 330.0f;
constexpr float kLevelBoxY = 390.0f;
constexpr float kLevelBoxW = 105.0f;
constexpr float kLevelBoxH = 30.0f;

constexpr float kXpBoxX = 447.0f;
constexpr float kXpBoxY = 395.0f;
constexpr float kXpBoxW = 240.0f;
constexpr float kXpBoxH = 12.0f;

constexpr float kStageBoxX = 138.0f;
constexpr float kStageBoxY = 202.0f;
constexpr float kStageBoxW = 200.0f;
constexpr float kStageBoxH = 40.0f;

constexpr float kDisabledBattleTextBoxX = 485.0f;
constexpr float kDisabledBattleTextBoxY = 182.0f;
constexpr float kDisabledBattleTextBoxW = 162.0f;
constexpr float kDisabledBattleTextBoxH = 30.0f;

constexpr float kAchvButtonY = 311.0f;
constexpr float kAchvButtonSpacing = 70.0f;
constexpr float kAchvButtonW = 64.0f;
constexpr float kAchvButtonH = 64.0f;

constexpr float kPopupCenterX = 400.0f;
constexpr float kPopupCenterY = 200.0f;
constexpr float kPopupBgW = 512.0f;
constexpr float kPopupBgH = 256.0f;
constexpr float kPopupLogoCenterX = 80.0f;
constexpr float kPopupLogoCenterY = 143.0f;
constexpr float kPopupBarX = 61.0f;
constexpr float kPopupBarY = 55.0f;
constexpr float kPopupBarFillOffsetX = 1.0f;
constexpr float kPopupTitleX = 166.0f;
constexpr float kPopupTitleY = 155.0f;
constexpr float kPopupDescX = 166.0f;
constexpr float kPopupDescY = 106.0f;
constexpr float kPopupRewardX = 60.0f;
constexpr float kPopupRewardY = 27.0f;
constexpr float kPopupBarScaleMax = 42.5f;

// Editable layout tweaks for the online-data screen after the main menu.
static std::array<LayoutTweak, static_cast<int>(OnlineDataLayoutId::Count)> kOnlineDataLayoutTweaks = {{
    {0.0f, -20.0f, 1.0f}, // Player name
    {0.0f, -20.0f, 1.0f}, // Level
    {0.0f, -10.0f, 1.0f}, // numbers X/Y of xp
    {0.0f, 0.0f, 1.0f}, // Level progression
    {0.0f, -20.0f, 1.0f}, // Stage
}};

const LayoutTweak& online_data_layout_tweak(OnlineDataLayoutId id) {
    return kOnlineDataLayoutTweaks[static_cast<int>(id)];
}

const char* bg_path() {
    return Param::language == 1 ? kBgKr : kBg;
}

const char* single_up_path() {
    return kSingleUp;
}

const char* single_down_path() {
    return kSingleDown;
}

const char* battle_up_path() {
    return Param::language == 1 ? kBattleUpKr : kBattleUp;
}

const char* battle_down_path() {
    return Param::language == 1 ? kBattleDownKr : kBattleDown;
}

float texture_width(const char* path, float fallback_w) {
    const auto& tex = TextureCache::instance().get(path);
    return (tex.valid && tex.width > 0) ? static_cast<float>(tex.width) : fallback_w;
}

float texture_height(const char* path, float fallback_h) {
    const auto& tex = TextureCache::instance().get(path);
    return (tex.valid && tex.height > 0) ? static_cast<float>(tex.height) : fallback_h;
}

std::vector<std::string> split_lines(std::string_view text) {
    std::vector<std::string> lines;
    size_t start = 0;
    while (start <= text.size()) {
        const size_t end = text.find('\n', start);
        if (end == std::string_view::npos) {
            lines.emplace_back(text.substr(start));
            break;
        }
        lines.emplace_back(text.substr(start, end - start));
        start = end + 1;
    }
    if (lines.empty()) {
        lines.emplace_back();
    }
    return lines;
}

} // namespace

OnlineDataScene::OnlineDataScene(TransitionRequest transition_cb, SetOnlineModeRequest set_online_mode_cb)
    : transition_cb_(std::move(transition_cb)), set_online_mode_cb_(std::move(set_online_mode_cb)) {
    reset();
}

bool OnlineDataScene::hit_test(const char* path,
                               float button_x,
                               float button_y,
                               float fallback_w,
                               float fallback_h,
                               float point_x,
                               float point_y) {
    const float x0 = Scene::get_x(button_x);
    const float y0 = Scene::get_y(button_y);
    const float x1 = Scene::get_x(button_x + texture_width(path, fallback_w));
    const float y1 = Scene::get_y(button_y + texture_height(path, fallback_h));
    return point_x >= x0 && point_x <= x1 && point_y >= y0 && point_y <= y1;
}

void OnlineDataScene::draw_sprite(const char* path,
                                  float x,
                                  float y,
                                  float fallback_w,
                                  float fallback_h,
                                  float alpha) {
    const auto& tex = TextureCache::instance().get(path);
    const float base_w = texture_width(path, fallback_w);
    const float base_h = texture_height(path, fallback_h);
    const float draw_x = get_x(x);
    const float draw_y = get_y(y);
    const float draw_w = get_x(base_w);
    const float draw_h = get_y(base_h);

    if (tex.valid) {
        draw_textured_quad(tex.id, draw_x, draw_y, draw_w, draw_h, alpha);
    } else {
        draw_quad(draw_x, draw_y, draw_w, draw_h, 0.2f, 0.2f, 0.22f, alpha);
    }
}

void draw_background_cover(const char* path) {
    const auto& tex = TextureCache::instance().get(path);
    if (tex.valid) {
        draw_textured_quad(tex.id, 0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 1.0f);
    } else {
        draw_quad(0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 0.08f, 0.12f, 0.18f, 1.0f);
    }
}

void draw_centered_sprite(const char* path,
                          float cx,
                          float cy,
                          float fallback_w,
                          float fallback_h,
                          float scale,
                          float alpha) {
    const auto& tex = TextureCache::instance().get(path);
    const float draw_w = Scene::get_x(texture_width(path, fallback_w)) * scale;
    const float draw_h = Scene::get_y(texture_height(path, fallback_h)) * scale;
    const float draw_x = Scene::get_x(cx) - (draw_w * 0.5f);
    const float draw_y = Scene::get_y(cy) - (draw_h * 0.5f);
    if (tex.valid) {
        draw_textured_quad(tex.id, draw_x, draw_y, draw_w, draw_h, alpha);
    } else {
        draw_quad(draw_x, draw_y, draw_w, draw_h, 0.2f, 0.2f, 0.22f, alpha);
    }
}

void OnlineDataScene::draw_bar_piece(const char* path,
                                     float x,
                                     float y,
                                     float piece_w,
                                     float piece_h,
                                     float scale_x,
                                     float alpha) {
    const auto& tex = TextureCache::instance().get(path);
    const float draw_x = get_x(x);
    const float draw_y = get_y(y);
    const float draw_w = get_x(piece_w * scale_x);
    const float draw_h = get_y(piece_h);

    if (tex.valid) {
        draw_textured_quad(tex.id, draw_x, draw_y, draw_w, draw_h, alpha);
    } else {
        draw_quad(draw_x, draw_y, draw_w, draw_h, 0.15f, 0.15f, 0.15f, alpha);
    }
}

bool hit_test_box(float x, float y, float w, float h, float point_x, float point_y) {
    const float x0 = Scene::get_x(x);
    const float y0 = Scene::get_y(y);
    const float x1 = Scene::get_x(x + w);
    const float y1 = Scene::get_y(y + h);
    return point_x >= x0 && point_x <= x1 && point_y >= y0 && point_y <= y1;
}

float achievement_button_x(int index) {
    return 123.0f + (kAchvButtonSpacing * static_cast<float>(index));
}

const char* achievement_logo_path(int type, int level) {
    static constexpr std::array<std::array<const char*, 4>, 8> kLogos = {{
        {{"assets/imgs_480_800/achieve/achieve_logo_gold_0.png", "assets/imgs_480_800/achieve/achieve_logo_gold_1.png", "assets/imgs_480_800/achieve/achieve_logo_gold_2.png", "assets/imgs_480_800/achieve/achieve_logo_gold_3.png"}},
        {{"assets/imgs_480_800/achieve/achieve_logo_stone_0.png", "assets/imgs_480_800/achieve/achieve_logo_stone_1.png", "assets/imgs_480_800/achieve/achieve_logo_stone_2.png", "assets/imgs_480_800/achieve/achieve_logo_stone_3.png"}},
        {{"assets/imgs_480_800/achieve/achieve_logo_kill_0.png", "assets/imgs_480_800/achieve/achieve_logo_kill_1.png", "assets/imgs_480_800/achieve/achieve_logo_kill_2.png", "assets/imgs_480_800/achieve/achieve_logo_kill_3.png"}},
        {{"assets/imgs_480_800/achieve/achieve_logo_stage_0.png", "assets/imgs_480_800/achieve/achieve_logo_stage_1.png", "assets/imgs_480_800/achieve/achieve_logo_stage_2.png", "assets/imgs_480_800/achieve/achieve_logo_stage_3.png"}},
        {{"assets/imgs_480_800/achieve/achieve_logo_ring_1.png", "assets/imgs_480_800/achieve/achieve_logo_ring_2.png", "assets/imgs_480_800/achieve/achieve_logo_ring_3.png", "assets/imgs_480_800/achieve/achieve_logo_ring_4.png"}},
        {{kAchvCast0, "assets/imgs_480_800/achieve/achieve_logo_fire_1.png", "assets/imgs_480_800/achieve/achieve_logo_fire_2.png", "assets/imgs_480_800/achieve/achieve_logo_fire_3.png"}},
        {{kAchvCast0, "assets/imgs_480_800/achieve/achieve_logo_ice_1.png", "assets/imgs_480_800/achieve/achieve_logo_ice_2.png", "assets/imgs_480_800/achieve/achieve_logo_ice_3.png"}},
        {{kAchvCast0, "assets/imgs_480_800/achieve/achieve_logo_lightning_1.png", "assets/imgs_480_800/achieve/achieve_logo_lightning_2.png", "assets/imgs_480_800/achieve/achieve_logo_lightning_3.png"}}
    }};
    return kLogos[static_cast<size_t>(std::clamp(type, 0, 7))][static_cast<size_t>(std::clamp(level, 0, 3))];
}

std::string achievement_title(int type, int level) {
    static constexpr std::array<const char*, 8> kPrefix = {{
        "Big Spender Lv.", "Great Mage Lv.", "Monster Hunter Lv.", "Defender Lv.",
        "Tactician Master Lv.", "Fire Master Lv.", "Ice Master Lv.", "Lightning Master Lv."
    }};
    return std::string(kPrefix[static_cast<size_t>(std::clamp(type, 0, 7))]) + std::to_string(std::max(0, level));
}

std::string achievement_progress_text(int type, int level, int amount) {
    const int max_amount = AchvData::get_max_amount(type, level);
    if (level >= 3 && amount >= max_amount) {
        return "Max";
    }
    return std::to_string(std::min(amount, max_amount)) + "/" + std::to_string(max_amount);
}

std::string achievement_next_text(int type, int level) {
    if (level >= 3) {
        return "Level Max";
    }
    const int target = AchvData::get_max_amount(type, level);
    switch (type) {
    case AchvData::GOLD: return "Next Level:\nCost " + std::to_string(target) + " coins.";
    case AchvData::STONE: return "Next Level:\nUse " + std::to_string(target) + " crystals for recovering mana.";
    case AchvData::KILL: return "Next Level:\nKill " + std::to_string(target) + " monsters.";
    case AchvData::STAGE: return "Next Level:\nComplete stage " + std::to_string(target) + ".";
    case AchvData::WIN: return "Next Level:\nWin " + std::to_string(target) + " battles.";
    case AchvData::FIRE: return "Next Level:\nCast fire spell " + std::to_string(target) + " times.";
    case AchvData::ICE: return "Next Level:\nCast ice spell " + std::to_string(target) + " times.";
    case AchvData::LIGHT: return "Next Level:\nCast lightning spell " + std::to_string(target) + " times.";
    default: return {};
    }
}

std::string achievement_reward_text(int type, int level) {
    if (level <= 0) {
        return {};
    }
    const int reward = AchvData::get_reward(type, level);
    switch (type) {
    case AchvData::GOLD: return "+" + std::to_string(reward) + " extra coin each kill";
    case AchvData::STONE: return "+" + std::to_string(reward) + "% crystal's mana recover";
    case AchvData::KILL: return "+" + std::to_string(reward) + "% bow's damage";
    case AchvData::STAGE: return "+" + std::to_string(reward) + "% extra XP in Local";
    case AchvData::WIN: return "+" + std::to_string(reward) + "% extra XP in Battle";
    case AchvData::FIRE: return "+" + std::to_string(reward) + "% fire's damage";
    case AchvData::ICE: return "+" + std::to_string(reward) + "% ice's damage";
    case AchvData::LIGHT: return "+" + std::to_string(reward) + "% lightning's damage";
    default: return {};
    }
}

void draw_text_box(FontRenderer& fonts,
                   FontFaceId font,
                   const std::string& text,
                   float x,
                   float y,
                   float w,
                   float h,
                   TextAlign align,
                   float r,
                   float g,
                   float b,
                   float a,
                   bool shadow = false) {
    float draw_x = Scene::get_x(x);
    const float draw_y = Scene::get_y(y + h);
    const float text_w = fonts.measure_text_width(font, text, Scene::get_y(h));
    if (align == TextAlign::Center) {
        draw_x += (Scene::get_x(w) - text_w) * 0.5f;
    } else if (align == TextAlign::Right) {
        draw_x += Scene::get_x(w) - text_w;
    }
    if (shadow) {
        fonts.draw_text(font, text, draw_x + Scene::get_x(1.0f), draw_y + Scene::get_y(1.0f), Scene::get_y(h), 0.0f, 0.0f, 0.0f, a);
    }
    fonts.draw_text(font, text, draw_x, draw_y, Scene::get_y(h), r, g, b, a);
}

void draw_text_layout_box(FontRenderer& fonts,
                          FontFaceId font,
                          const std::string& text,
                          float x,
                          float y,
                          float w,
                          float h,
                          float font_size,
                          TextAlign align,
                          float r,
                          float g,
                          float b,
                          float a,
                          bool shadow = false) {
    const float draw_h = Scene::get_y(font_size);
    float draw_x = Scene::get_x(x);
    const float draw_y = Scene::get_y(y + ((h - font_size) * 0.5f) + font_size);
    const float text_w = fonts.measure_text_width(font, text, draw_h);
    if (align == TextAlign::Center) {
        draw_x += std::max(0.0f, (Scene::get_x(w) - text_w) * 0.5f);
    } else if (align == TextAlign::Right) {
        draw_x += std::max(0.0f, Scene::get_x(w) - text_w);
    }
    if (shadow) {
        fonts.draw_text(font, text, draw_x + Scene::get_x(1.0f), draw_y + Scene::get_y(1.0f), draw_h, 0.0f, 0.0f, 0.0f, a);
    }
    fonts.draw_text(font, text, draw_x, draw_y, draw_h, r, g, b, a);
}

void draw_text_layout_box_with_layout(FontRenderer& fonts,
                                      FontFaceId font,
                                      const std::string& text,
                                      float box_x,
                                      float box_y,
                                      float box_w,
                                      float box_h,
                                      float font_h,
                                      TextAlign align,
                                      float r,
                                      float g,
                                      float b,
                                      float a,
                                      OnlineDataLayoutId layout_id,
                                      bool shadow = false) {
    const LayoutTweak& layout = online_data_layout_tweak(layout_id);
    draw_text_layout_box(
        fonts,
        font,
        text,
        box_x + layout.x,
        box_y + layout.y,
        box_w,
        box_h,
        font_h * layout.scale,
        align,
        r,
        g,
        b,
        a,
        shadow
    );
}

void draw_multiline_box(FontRenderer& fonts,
                        FontFaceId font,
                        const std::string& text,
                        float x,
                        float y,
                        float w,
                        float font_h,
                        float line_spacing,
                        TextAlign align,
                        float r,
                        float g,
                        float b,
                        float a,
                        bool shadow = false) {
    float pen_y = y;
    for (const std::string& line : split_lines(text)) {
        draw_text_box(fonts, font, line, x, pen_y, w, font_h, align, r, g, b, a, shadow);
        pen_y += font_h + line_spacing;
    }
}

void draw_multiline_layout_box(FontRenderer& fonts,
                               FontFaceId font,
                               const std::string& text,
                               float x,
                               float y,
                               float w,
                               float h,
                               float font_size,
                               float line_spacing,
                               TextAlign align,
                               float r,
                               float g,
                               float b,
                               float a,
                               bool shadow = false) {
    const auto lines = split_lines(text);
    const float block_h = (static_cast<float>(lines.size()) * font_size) +
                          (static_cast<float>(std::max(0, static_cast<int>(lines.size()) - 1)) * line_spacing);
    float pen_y = y + std::max(0.0f, (h - block_h) * 0.5f);
    for (const std::string& line : lines) {
        draw_text_layout_box(fonts, font, line, x, pen_y, w, font_size, font_size, align, r, g, b, a, shadow);
        pen_y += font_size + line_spacing;
    }
}

bool OnlineDataScene::touch(const TouchEvent& event) {
    if (HelpOverlay::touch(event, event.x1, event.y1)) {
        return true;
    }

    if (event.action == TouchAction::Down) {
        for (int i = 0; i < static_cast<int>(achv_pressed_.size()); ++i) {
            if (hit_test_box(achievement_button_x(i), kAchvButtonY, kAchvButtonW, kAchvButtonH, event.x1, event.y1) &&
                (i != achv_selected_ || !achv_pressed_[static_cast<size_t>(achv_selected_)])) {
                achv_pressed_.fill(false);
                achv_selected_ = i;
                achv_pressed_[static_cast<size_t>(i)] = true;
                achv_popup_type_ = i;
                achv_popup_level_ = achv_levels_[static_cast<size_t>(i)];
                achv_popup_amount_ = achv_amounts_[static_cast<size_t>(i)];
                achv_popup_show_ = true;
                return true;
            }
        }
        if (achv_pressed_[static_cast<size_t>(achv_selected_)]) {
            achv_pressed_[static_cast<size_t>(achv_selected_)] = false;
            achv_popup_show_ = false;
            return true;
        }

        pressed_button_ = PressedButton::None;
        if (hit_test(single_up_path(), kSingleX, kSingleY, 0.0f, 0.0f, event.x1, event.y1)) {
            pressed_button_ = PressedButton::Single;
            return true;
        }
        if (hit_test_box(kNameBoxX, kNameBoxY, kNameBoxW, kNameBoxH, event.x1, event.y1)) {
            pending_name_before_edit_ = Param::player_name;
            name_edit_pending_ = true;
            Param::request_name_edit = true;
            return true;
        }
        return true;
    }

    if (event.action == TouchAction::Up) {
        const PressedButton released = pressed_button_;
        pressed_button_ = PressedButton::None;

        if (released == PressedButton::Single &&
            hit_test(single_up_path(), kSingleX, kSingleY, 0.0f, 0.0f, event.x1, event.y1)) {
            AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
            Param::is_online_mode = false;
            if (set_online_mode_cb_) {
                set_online_mode_cb_(false);
            }
            AbstractGame::reset_game_time();
            if (transition_cb_) {
                transition_cb_(Param::stage == 1 ? Param::SCENE_RESEARCH : Param::SCENE_MAIN_GAME, 0);
            }
            return true;
        }
        return true;
    }

    return true;
}

void OnlineDataScene::draw() {
    draw_background_cover(bg_path());

    draw_sprite(
        pressed_button_ == PressedButton::Single ? single_down_path() : single_up_path(),
        kSingleX,
        kSingleY,
        0.0f,
        0.0f,
        1.0f
    );

    draw_sprite(
        battle_up_path(),
        kBattleX,
        kBattleY,
        0.0f,
        0.0f,
        0.16f
    );

    const LayoutTweak& level_progress_layout = online_data_layout_tweak(OnlineDataLayoutId::LevelProgress);
    draw_bar_piece(
        kExPanelBg,
        kPieceX + level_progress_layout.x,
        kPieceY + level_progress_layout.y,
        kPieceW * level_progress_layout.scale,
        kPieceH * level_progress_layout.scale,
        kPieceScaleX * level_progress_layout.scale,
        1.0f
    );
    draw_bar_piece(
        kExPiece,
        kPieceX + level_progress_layout.x,
        kPieceY + level_progress_layout.y,
        kPieceW * level_progress_layout.scale,
        kPieceH * level_progress_layout.scale,
        kPieceScaleX * shown_exp_ratio_ * level_progress_layout.scale,
        1.0f
    );
    draw_sprite(
        kExPanel,
        kPanelX + level_progress_layout.x,
        kPanelY + level_progress_layout.y,
        257.0f * level_progress_layout.scale,
        22.0f * level_progress_layout.scale,
        1.0f
    );

    for (int i = 0; i < static_cast<int>(achv_levels_.size()); ++i) {
        const float button_x = achievement_button_x(i);
        const float alpha = achv_pressed_[static_cast<size_t>(i)] ? 0.7f : 1.0f;
        draw_sprite(kAchvLogoBg, button_x, kAchvButtonY, kAchvButtonW, kAchvButtonH, alpha);

        const char* logo_path = achievement_logo_path(i, achv_levels_[static_cast<size_t>(i)]);
        const float logo_w = texture_width(logo_path, 50.0f);
        const float logo_h = texture_height(logo_path, 50.0f);
        draw_sprite(
            logo_path,
            button_x + ((kAchvButtonW - logo_w) * 0.5f),
            kAchvButtonY + ((kAchvButtonH - logo_h) * 0.5f),
            logo_w,
            logo_h,
            alpha
        );
    }

    auto& fonts = FontRenderer::instance();
    char line[96];
    draw_text_layout_box_with_layout(
        fonts,
        FontFaceId::Cooper,
        Param::player_name,
        kNameBoxX,
        kNameBoxY,
        kNameBoxW,
        kNameBoxH,
        22.0f,
        TextAlign::Center,
        0.95f,
        0.92f,
        0.84f,
        0.98f,
        OnlineDataLayoutId::PlayerName,
        true
    );

    std::snprintf(line, sizeof(line), "Lv. %d", std::max(1, Param::level));
    draw_text_layout_box_with_layout(
        fonts,
        FontFaceId::Cooper,
        line,
        kLevelBoxX,
        kLevelBoxY,
        kLevelBoxW,
        kLevelBoxH,
        22.0f,
        TextAlign::Center,
        0.95f,
        0.92f,
        0.84f,
        0.98f,
        OnlineDataLayoutId::Level,
        true
    );

    const int shown_xp = std::min(Param::xp, static_cast<int>(static_cast<float>(exp_max_) * shown_exp_ratio_));
    std::snprintf(line, sizeof(line), "%d/%d", shown_xp, std::max(1, exp_max_));
    draw_text_layout_box_with_layout(
        fonts,
        FontFaceId::Ants,
        line,
        kXpBoxX,
        kXpBoxY,
        kXpBoxW,
        kXpBoxH,
        14.0f,
        TextAlign::Center,
        1.0f,
        1.0f,
        1.0f,
        0.98f,
        OnlineDataLayoutId::XpNumbers,
        true
    );

    std::snprintf(line, sizeof(line), "Stage  %d", std::max(1, Param::stage));
    draw_text_layout_box_with_layout(
        fonts,
        FontFaceId::Cooper,
        line,
        kStageBoxX,
        kStageBoxY,
        kStageBoxW,
        kStageBoxH,
        23.0f,
        TextAlign::Center,
        1.0f,
        0.95f,
        0.76f,
        0.98f,
        OnlineDataLayoutId::Stage,
        true
    );

    draw_multiline_layout_box(
        fonts,
        FontFaceId::Ants,
        "Disabled\nOnline",
        kDisabledBattleTextBoxX,
        kDisabledBattleTextBoxY,
        kDisabledBattleTextBoxW,
        kDisabledBattleTextBoxH,
        14.0f,
        0.0f,
        TextAlign::Center,
        0.46f,
        0.18f,
        0.18f,
        0.85f,
        true
    );

    if (achv_popup_alpha_ > 0.0f) {
        const float popup_left = kPopupCenterX - (kPopupBgW * 0.5f);
        const float popup_top = kPopupCenterY - (kPopupBgH * 0.5f);
        draw_centered_sprite(kAchvPanelBg, kPopupCenterX, kPopupCenterY, kPopupBgW, kPopupBgH, 0.8f + (achv_popup_alpha_ / 5.0f), achv_popup_alpha_);

        const char* logo_path = achievement_logo_path(achv_popup_type_, achv_popup_level_);
        draw_centered_sprite(logo_path, popup_left + kPopupLogoCenterX, popup_top + kPopupLogoCenterY, 50.0f, 50.0f, 1.0f, achv_popup_alpha_);

        draw_sprite(kAchvPieceBg, popup_left + kPopupBarX, popup_top + kPopupBarY, 340.0f, 22.0f, achv_popup_alpha_);
        const int popup_bar_max = AchvData::get_max_amount(achv_popup_type_, achv_popup_level_);
        const float popup_fill_scale = popup_bar_max > 0
            ? std::min(static_cast<float>(achv_popup_amount_), static_cast<float>(popup_bar_max)) * kPopupBarScaleMax / static_cast<float>(popup_bar_max)
            : 0.0f;
        draw_bar_piece(
            kAchvPiece,
            popup_left + kPopupBarX + kPopupBarFillOffsetX,
            popup_top + kPopupBarY,
            texture_width(kAchvPiece, 8.0f),
            texture_height(kAchvPiece, 20.0f),
            popup_fill_scale,
            achv_popup_alpha_
        );
        draw_sprite(kAchvPieceFrame, popup_left + kPopupBarX - 1.0f, popup_top + kPopupBarY, 340.0f, 22.0f, achv_popup_alpha_);

        draw_text_box(
            fonts,
            FontFaceId::Cooper,
            achievement_progress_text(achv_popup_type_, achv_popup_level_, achv_popup_amount_),
            popup_left + kPopupBarX,
            popup_top + kPopupBarY + 1.0f,
            340.0f,
            18.0f,
            TextAlign::Center,
            0.0f,
            0.0f,
            0.0f,
            achv_popup_alpha_,
            true
        );
        draw_text_box(
            fonts,
            FontFaceId::Cooper,
            achievement_title(achv_popup_type_, achv_popup_level_),
            popup_left + kPopupTitleX,
            popup_top + kPopupTitleY,
            240.0f,
            24.0f,
            TextAlign::Center,
            1.0f,
            0.96f,
            0.74f,
            achv_popup_alpha_,
            true
        );
        draw_multiline_box(
            fonts,
            FontFaceId::Ants,
            achievement_next_text(achv_popup_type_, achv_popup_level_),
            popup_left + kPopupDescX,
            popup_top + kPopupDescY,
            270.0f,
            18.0f,
            -2.0f,
            TextAlign::Left,
            1.0f,
            1.0f,
            1.0f,
            achv_popup_alpha_,
            true
        );
        const std::string reward = achievement_reward_text(achv_popup_type_, std::max(achv_popup_level_, 1));
        if (!reward.empty()) {
            draw_text_box(
                fonts,
                FontFaceId::Ants,
                reward,
                popup_left + kPopupRewardX,
                popup_top + kPopupRewardY,
                360.0f,
                18.0f,
                TextAlign::Left,
                0.2f,
                1.0f,
                0.2f,
                achv_popup_alpha_,
                true
            );
        }
    }

    HelpOverlay::draw();
}

void OnlineDataScene::update() {
    if (shown_exp_ratio_ < target_exp_ratio_) {
        shown_exp_ratio_ += static_cast<float>(AbstractGame::last_span_ms()) / 2000.0f;
        if (shown_exp_ratio_ > target_exp_ratio_) {
            shown_exp_ratio_ = target_exp_ratio_;
        }
    }

    const float popup_delta = static_cast<float>(AbstractGame::last_span_ms()) / 100.0f;
    if (achv_popup_show_) {
        achv_popup_alpha_ = std::min(1.0f, achv_popup_alpha_ + popup_delta);
    } else if (achv_popup_alpha_ > 0.0f) {
        achv_popup_alpha_ = std::max(0.0f, achv_popup_alpha_ - popup_delta);
    }

    if (name_edit_pending_ && Param::player_name != pending_name_before_edit_) {
        HelpOverlay::active();
        name_edit_pending_ = false;
    }

    HelpOverlay::update();
}

void OnlineDataScene::reset() {
    pressed_button_ = PressedButton::None;
    Param::xp = Save::load_data(Save::XP);
    Param::level = Save::load_data(Save::LEVEL);
    Param::win = Save::load_data(Save::WIN);
    Param::lose = Save::load_data(Save::LOSE);
    Param::stage = Save::load_data(Save::STAGE);
    Param::bt_level = Save::load_data(Save::BTL_LEVEL);
    Param::bt_time = Save::load_data(Save::BTL_TIME);
    Param::cast_fire = Save::load_data(Save::FIRE_CAST);
    Param::cast_ice = Save::load_data(Save::ICE_CAST);
    Param::cast_light = Save::load_data(Save::LIGHT_CAST);
    Param::total_kills = Save::load_data(Save::KILLS);
    Param::cost_coin = Save::load_data(Save::COST_COIN);
    Param::cost_stone = Save::load_data(Save::COST_STONE);
    Param::player_name = Save::load_name();
    if (Param::level < 1) Param::level = 1;
    if (Param::stage < 1) Param::stage = 1;

    for (int i = 0; i < static_cast<int>(achv_amounts_.size()); ++i) {
        achv_amounts_[static_cast<size_t>(i)] = AchvData::get_cur_amount(i);
        achv_levels_[static_cast<size_t>(i)] = AchvData::get_level(i, achv_amounts_[static_cast<size_t>(i)]);
    }
    achv_pressed_.fill(false);
    achv_selected_ = 0;
    achv_popup_show_ = false;
    achv_popup_alpha_ = 0.0f;
    achv_popup_type_ = 0;
    achv_popup_level_ = achv_levels_[0];
    achv_popup_amount_ = achv_amounts_[0];

    exp_max_ = XpData::get_max_xp(std::max(1, Param::level));
    target_exp_ratio_ = std::clamp(static_cast<float>(Param::xp) / static_cast<float>(std::max(1, exp_max_)), 0.0f, 1.0f);
    shown_exp_ratio_ = 0.0f;

    pending_name_before_edit_ = Param::player_name;
    name_edit_pending_ = false;

    HelpOverlay::set_help(1);
    if (Param::level >= 2) {
        HelpOverlay::set_help(7);
    }

    scene_x = 0.0f;
    scene_y = 0.0f;
    scene_scale = 1.0f;
    scene_alpha = 1.0f;
}

} // namespace defender
