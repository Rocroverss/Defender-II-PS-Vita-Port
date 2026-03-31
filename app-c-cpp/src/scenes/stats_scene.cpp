#include "scenes/stats_scene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <utility>

#include "engine/abstract_game.hpp"
#include "engine/font_renderer.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/achv_mng.hpp"
#include "game/audio_manager.hpp"
#include "game/item_param.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "game/skill_data.hpp"
#include "game/sounds.hpp"
#include "game/umeng_helper.hpp"
#include "game/xp_data.hpp"

namespace defender {
namespace {

constexpr const char* kBgLocal = "assets/imgs_480_800/stats/stats_local_bg.jpg";
constexpr const char* kBgOnline = "assets/imgs_480_800/stats/stats_online_bg.jpg";
constexpr const char* kBgOnlineKr = "assets/imgs_480_800/stats/stats_online_bg_kr.jpg";
constexpr const char* kFgWin = "assets/imgs_480_800/stats/stats_win_fg.png";
constexpr const char* kFgWinKr = "assets/imgs_480_800/stats/stats_win_fg_kr.png";
constexpr const char* kFgLose = "assets/imgs_480_800/stats/stats_failed_fg.png";
constexpr const char* kFgLoseKr = "assets/imgs_480_800/stats/stats_failed_fg_kr.png";
constexpr const char* kWinStamp = "assets/imgs_480_800/stats/stats_win_stamp.png";
constexpr const char* kContinue = "assets/imgs_480_800/game/gameover_tips.png";
constexpr const char* kReportUp = "assets/imgs_480_800/stats/button_report_up.png";
constexpr const char* kReportDown = "assets/imgs_480_800/stats/button_report_down.png";
constexpr const char* kStatsBonus = "assets/imgs_480_800/stats/stats_piece_bonus.png";
constexpr const char* kStatsBonusKr = "assets/imgs_480_800/stats/stats_piece_bonus_kr.png";
constexpr const char* kStatsLife = "assets/imgs_480_800/stats/stats_piece_life.png";
constexpr const char* kStatsLifeKr = "assets/imgs_480_800/stats/stats_piece_life_kr.png";
constexpr const char* kStatsKills = "assets/imgs_480_800/stats/stats_piece_kills.png";
constexpr const char* kStatsKillsKr = "assets/imgs_480_800/stats/stats_piece_kills_kr.png";
constexpr const char* kStatsXp = "assets/imgs_480_800/stats/stats_piece_xp.png";

constexpr const char* kExPanel = "assets/imgs_480_800/onlinedata/ex_panel.png";
constexpr const char* kExPiece = "assets/imgs_480_800/onlinedata/ex_piece.png";
constexpr const char* kExPieceBg = "assets/imgs_480_800/onlinedata/ex_piece_bg.png";

constexpr float kReportX = 635.0f;
constexpr float kReportY = 340.0f;
constexpr float kReportW = 136.0f;
constexpr float kReportH = 39.0f;

constexpr float kFgWinW = 277.0f;
constexpr float kFgWinH = 33.0f;
constexpr float kFgLoseW = 230.0f;
constexpr float kFgLoseH = 32.0f;
constexpr float kWinStampW = 148.0f;
constexpr float kWinStampH = 56.0f;

constexpr float kPieceLabelW = 349.0f;
constexpr float kPieceLabelH = 33.0f;
constexpr float kXpPanelW = 257.0f;
constexpr float kXpPanelH = 22.0f;
constexpr float kXpBarW = 248.0f;
constexpr float kXpBarH = 14.0f;

constexpr float kLocalLabelX = 220.0f;
constexpr float kLocalBonusLabelY = 90.0f;
constexpr float kLocalLifeLabelY = 147.0f;
constexpr float kLocalKillsLabelY = 194.0f;
constexpr float kLocalXpLabelY = 243.0f;
constexpr float kLocalXpPanelX = 335.0f;
constexpr float kLocalXpPanelY = 290.0f;
constexpr float kLocalXpFillX = 339.0f;
constexpr float kLocalXpFillY = 294.0f;
constexpr float kLocalLevelBoxX = 148.0f;
constexpr float kLocalLevelBoxY = 290.0f;
constexpr float kLocalLevelBoxW = 185.0f;
constexpr float kLocalLevelBoxH = 28.0f;
constexpr float kLocalXpBarTextX = 347.0f;
constexpr float kLocalXpBarTextY = 292.0f;
constexpr float kLocalXpBarTextW = 248.0f;
constexpr float kLocalXpBarTextH = 14.0f;
constexpr float kLocalRowValueX = 347.0f;
constexpr float kLocalRowValueW = 248.0f;
constexpr float kLocalXpGainY = 248.0f;
constexpr float kLocalXpGainH = 24.0f;
constexpr float kLocalKillsValueY = 198.0f;
constexpr float kLocalKillsValueH = 24.0f;
constexpr float kLocalLifeValueY = 148.0f;
constexpr float kLocalLifeValueH = 24.0f;
constexpr float kLocalBonusCoinValueX = 401.0f;
constexpr float kLocalBonusCoinValueY = 96.0f;
constexpr float kLocalBonusCoinValueW = 74.0f;
constexpr float kLocalBonusValueH = 24.0f;
constexpr float kLocalBonusStoneValueX = 521.0f;
constexpr float kLocalBonusStoneValueY = 96.0f;
constexpr float kLocalBonusStoneValueW = 48.0f;
constexpr float kLocalFgCenterX = 404.0f;
constexpr float kLocalFgCenterY = 365.0f;

constexpr float kColorDarkR = 24.0f / 255.0f;
constexpr float kColorDarkG = 44.0f / 255.0f;
constexpr float kColorDarkB = 60.0f / 255.0f;

struct ScreenOffset {
    float x;
    float y;
};

enum class StageCompleteTextOffsetId : int {
    Level = 0,
    XpBar,
    BonusGold,
    BonusStone,
    Life,
    Kills,
    XpGain,
    Continue,
    Count
};

// Editable offsets for the local stage-complete screen text positions.
static std::array<ScreenOffset, static_cast<int>(StageCompleteTextOffsetId::Count)> kStageCompleteTextOffsets = {{
    {-10.0f, -21.3f}, // Level
    {-5.0f, -12.7f}, // XP bar
    {0.0f, -20.0f}, // Bonus gold
    {0.0f, -20.0f}, // Bonus stone
    {0.0f, -19.5f}, // Life
    {0.0f, -18.5f}, // Kills
    {0.0f, -20.0f}, // XP gain
    {50.0f, -432.0f}, // touch screen to continue
}};

enum class TextAlign {
    Left,
    Center,
    Right
};

const ScreenOffset& stage_complete_text_offset(StageCompleteTextOffsetId id) {
    return kStageCompleteTextOffsets[static_cast<int>(id)];
}

void draw_background_cover(const char* path) {
    const auto& tex = TextureCache::instance().get(path);
    if (tex.valid) {
        draw_textured_quad(tex.id, 0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 1.0f);
    } else {
        draw_quad(0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 0.10f, 0.10f, 0.12f, 1.0f);
    }
}

void draw_sprite(const char* path, float x, float y, float w, float h, float alpha = 1.0f) {
    const auto& tex = TextureCache::instance().get(path);
    const float dx = Scene::get_x(x);
    const float dy = Scene::get_y(y);
    const float dw = Scene::get_x(w);
    const float dh = Scene::get_y(h);
    if (tex.valid) {
        draw_textured_quad(tex.id, dx, dy, dw, dh, alpha);
    } else {
        draw_quad(dx, dy, dw, dh, 0.2f, 0.2f, 0.2f, alpha);
    }
}

void draw_sprite_centered(const char* path, float cx, float cy, float w, float h, float alpha = 1.0f) {
    draw_sprite(path, cx - (w * 0.5f), cy - (h * 0.5f), w, h, alpha);
}

bool contains_designed_rect(float px, float py, float x, float y, float w, float h) {
    const float left = Scene::get_x(x);
    const float top = Scene::get_y(y);
    const float right = left + Scene::get_x(w);
    const float bottom = top + Scene::get_y(h);
    return px >= left && px <= right && py >= top && py <= bottom;
}

void draw_text_box(
    FontRenderer& fonts,
    FontFaceId font,
    const std::string& text,
    float box_x,
    float box_y,
    float box_w,
    float font_size,
    TextAlign align,
    float r,
    float g,
    float b,
    float a,
    bool shadow = false
) {
    const float px_height = Scene::get_y(font_size);
    float draw_x = Scene::get_x(box_x);
    const float draw_y = Scene::get_y(box_y + font_size);
    const float box_w_px = Scene::get_x(box_w);
    const float text_w = fonts.measure_text_width(font, text, px_height);

    if (align == TextAlign::Center) {
        draw_x += std::max(0.0f, (box_w_px - text_w) * 0.5f);
    } else if (align == TextAlign::Right) {
        draw_x += std::max(0.0f, box_w_px - text_w);
    }

    if (shadow) {
        fonts.draw_text(font, text, draw_x + Scene::get_x(1.0f), draw_y + Scene::get_y(1.0f), px_height, 0.0f, 0.0f, 0.0f, a);
    }
    fonts.draw_text(font, text, draw_x, draw_y, px_height, r, g, b, a);
}

void draw_text_layout_box(
    FontRenderer& fonts,
    FontFaceId font,
    const std::string& text,
    float box_x,
    float box_y,
    float box_w,
    float box_h,
    float font_size,
    TextAlign align,
    float r,
    float g,
    float b,
    float a,
    bool shadow = false
) {
    const float px_height = Scene::get_y(font_size);
    float draw_x = Scene::get_x(box_x);
    const float draw_y = Scene::get_y(box_y + ((box_h - font_size) * 0.5f) + font_size);
    const float box_w_px = Scene::get_x(box_w);
    const float text_w = fonts.measure_text_width(font, text, px_height);

    if (align == TextAlign::Center) {
        draw_x += std::max(0.0f, (box_w_px - text_w) * 0.5f);
    } else if (align == TextAlign::Right) {
        draw_x += std::max(0.0f, box_w_px - text_w);
    }

    if (shadow) {
        fonts.draw_text(font, text, draw_x + Scene::get_x(1.0f), draw_y + Scene::get_y(1.0f), px_height, 0.0f, 0.0f, 0.0f, a);
    }
    fonts.draw_text(font, text, draw_x, draw_y, px_height, r, g, b, a);
}

void draw_text_layout_box_with_offset(
    FontRenderer& fonts,
    FontFaceId font,
    const std::string& text,
    float box_x,
    float box_y,
    float box_w,
    float box_h,
    float font_size,
    TextAlign align,
    float r,
    float g,
    float b,
    float a,
    StageCompleteTextOffsetId offset_id,
    bool shadow = false
) {
    const ScreenOffset& offset = stage_complete_text_offset(offset_id);
    draw_text_layout_box(
        fonts,
        font,
        text,
        box_x + offset.x,
        box_y + offset.y,
        box_w,
        box_h,
        font_size,
        align,
        r,
        g,
        b,
        a,
        shadow
    );
}

std::string level_word() {
    return "level";
}

void format_time(char* out, size_t out_size, int seconds) {
    std::snprintf(out, out_size, "%d:%02d", std::max(0, seconds) / 60, std::max(0, seconds) % 60);
}

} // namespace

StatsScene::StatsScene(TransitionRequest transition_cb) : transition_cb_(std::move(transition_cb)) {}

void StatsScene::refresh_level_text() {
    show_xp_max_ = XpData::get_max_xp(std::max(1, show_level_));
}

void StatsScene::apply_rewards() {
    is_win_ = Param::is_win;
    online_mode_ = Param::is_online_mode;
    count_done_ = false;
    press_flag_ = false;
    rate_flag_ = false;
    report_pressed_ = false;
    name_1_ = Save::load_name();
    name_2_ = "RIVAL";
    battle_level_ = std::max(1, Param::bt_level);
    time_1_sec_ = std::max(0, Param::time / 1000);
    time_2_sec_ = std::max(0, Param::rep_time / 1000);
    gold_bonus_ = 0;
    stone_bonus_ = 0;
    kill_bonus_ = std::max(0, Param::kills);
    life_bonus_ = std::max(0, Param::life_percent);
    xp_base_ = 0;
    xp_achv_ = 0;
    xp_skill_ = 0;

    if (online_mode_) {
        if (time_1_sec_ >= 180 && Param::bt_level < 15) {
            ++Param::bt_level;
            Save::save_data(Save::BTL_LEVEL, Param::bt_level);
            Param::bt_time = 0;
            Save::save_data(Save::BTL_TIME, 0);
        } else if (time_1_sec_ > Param::bt_time) {
            Param::bt_time = time_1_sec_;
            Save::save_data(Save::BTL_TIME, Param::bt_time);
        }

        is_win_ = time_1_sec_ >= time_2_sec_;
        if (is_win_) {
            ++Param::win;
            AchvMng::check_achv_in_game(4);
            AchvMng::push_battle_win_in_game(Param::win);
            Save::save_data(Save::WIN, Param::win);
            gold_bonus_ = static_cast<int>((((battle_level_ * 200.0f) * time_1_sec_) / 180.0f) + kill_bonus_);
            stone_bonus_ = 1;
            xp_base_ = static_cast<int>((((battle_level_ * 50.0f) * time_1_sec_) / 180.0f) + (kill_bonus_ / 2));
        } else {
            ++Param::lose;
            Save::save_data(Save::LOSE, Param::lose);
            gold_bonus_ = kill_bonus_;
            stone_bonus_ = 0;
            xp_base_ = static_cast<int>((((battle_level_ * 16.0f) * time_1_sec_) / 180.0f) + (kill_bonus_ / 2));
        }
        if (Param::extra_battle_xp > 0) {
            xp_achv_ = static_cast<int>((static_cast<float>(xp_base_) * Param::extra_battle_xp) / 100.0f) + 1;
        }
    } else {
        const int stage_before = std::max(1, Param::stage);
        if (is_win_) {
            UMengHelper::end_level(stage_before, true);
            if ((stage_before % 10) == 0) {
                gold_bonus_ = (stage_before * 17) + 50 + kill_bonus_;
                stone_bonus_ = 3;
                xp_base_ = (stage_before * 2) + 20 + (kill_bonus_ / 2);
            } else {
                gold_bonus_ = (stage_before * 7) + 50 + kill_bonus_;
                stone_bonus_ = 2;
                xp_base_ = stage_before + 20 + (kill_bonus_ / 2);
            }
            Param::stage = stage_before + 1;
            AchvMng::check_achv_in_game(3);
            AchvMng::push_stage_score_in_game(Param::stage);
            Save::save_data(Save::STAGE, Param::stage);
        } else {
            UMengHelper::end_level(stage_before, false);
            gold_bonus_ = kill_bonus_;
            stone_bonus_ = 0;
            xp_base_ = kill_bonus_ / 2;
        }
        if (Param::extra_local_xp > 0) {
            xp_achv_ = static_cast<int>((static_cast<float>(xp_base_) * Param::extra_local_xp) / 100.0f) + 1;
        }
    }

    if (ItemParam::get_level(ItemParam::SENIOR_HUNTER) > 0) {
        xp_skill_ = static_cast<int>((static_cast<float>(xp_base_) * SkillData::get_value(SkillData::SENIOR_HUNTER)) / 100.0f) + 1;
    }

    Param::gold += gold_bonus_;
    Param::stone += stone_bonus_;

    show_level_ = std::max(1, Param::level);
    show_xp_ = std::max(0, Param::xp);
    refresh_level_text();

    xp_pending_ = std::max(0, xp_base_ + xp_achv_ + xp_skill_);
    Param::xp += xp_pending_;
    while (Param::xp >= XpData::get_max_xp(Param::level)) {
        Param::xp -= XpData::get_max_xp(Param::level);
        ++Param::level;
    }

    Save::pause_save_data();
    Save::save_data(Save::LEVEL, Param::level);
    Save::save_data(Save::XP, Param::xp);
    Save::save_data(Save::WIN, Param::win);
    Save::save_data(Save::LOSE, Param::lose);
    Save::save_data(Save::BTL_LEVEL, Param::bt_level);
    Save::save_data(Save::BTL_TIME, Param::bt_time);

    xp_step_ = std::max(1, xp_pending_ / 50);
    xp_count_sound_cooldown_ms_ = 0;
    lv_alpha_ = 1.0f;
}

bool StatsScene::touch(const TouchEvent& event) {
    if (event.action == TouchAction::Down) {
        if (online_mode_ && contains_designed_rect(event.x1, event.y1, kReportX, kReportY, kReportW, kReportH)) {
            report_pressed_ = true;
        } else if (time_ms_ > 1000) {
            press_flag_ = true;
        }
        return true;
    }

    if (event.action == TouchAction::Up) {
        if (report_pressed_) {
            report_pressed_ = false;
            return true;
        }
        if (!count_done_) {
            return true;
        }
        if (transition_cb_) {
            if (Param::level > 1 && Save::load_data(std::string(Save::HELP) + "7") == 0) {
                transition_cb_(Param::SCENE_ONLINE_DATA, 0);
            } else {
                transition_cb_(Param::SCENE_RESEARCH, 0);
            }
        }
        return true;
    }
    return true;
}

void StatsScene::draw() {
    draw_background_cover(online_mode_ ? (Param::language == 1 ? kBgOnlineKr : kBgOnline) : kBgLocal);

    auto& fonts = FontRenderer::instance();
    char line[128];

    if (online_mode_) {
        draw_text_layout_box(fonts, FontFaceId::Cooper, name_1_, 184.0f, 345.0f, 185.0f, 35.0f, 24.0f, TextAlign::Center, kColorDarkR, kColorDarkG, kColorDarkB, 1.0f);
        draw_text_layout_box(fonts, FontFaceId::Cooper, name_2_, 432.0f, 345.0f, 185.0f, 35.0f, 24.0f, TextAlign::Center, kColorDarkR, kColorDarkG, kColorDarkB, 1.0f);

        format_time(line, sizeof(line), time_1_sec_);
        draw_text_layout_box(fonts, FontFaceId::Cooper, line, 202.0f, 265.0f, 150.0f, 60.0f, 38.0f, TextAlign::Center, 1.0f, 1.0f, 1.0f, 1.0f);
        if ((Param::single_battle_time / 1000) < time_2_sec_) {
            std::snprintf(line, sizeof(line), "??:??");
        } else {
            format_time(line, sizeof(line), time_2_sec_);
        }
        draw_text_layout_box(fonts, FontFaceId::Cooper, line, 450.0f, 265.0f, 150.0f, 60.0f, 38.0f, TextAlign::Center, 1.0f, 1.0f, 1.0f, 1.0f);

        std::snprintf(line, sizeof(line), "%s %d", level_word().c_str(), show_level_);
        draw_text_layout_box(fonts, FontFaceId::Cooper, line, 105.0f, 188.0f, 185.0f, 30.0f, 25.0f, TextAlign::Right, kColorDarkR, kColorDarkG, kColorDarkB, lv_alpha_);
        if (xp_skill_ > 0) {
            std::snprintf(line, sizeof(line), "%d +%d +%d", xp_base_, xp_achv_, xp_skill_);
        } else if (xp_achv_ > 0) {
            std::snprintf(line, sizeof(line), "%d +%d", xp_base_, xp_achv_);
        } else {
            std::snprintf(line, sizeof(line), "%d", xp_base_);
        }
        draw_text_layout_box(fonts, FontFaceId::Cooper, line, 325.0f, 150.0f, 250.0f, 26.0f, 22.0f, TextAlign::Left, 1.0f, 1.0f, 1.0f, 1.0f, true);
        std::snprintf(line, sizeof(line), "%d", gold_bonus_);
        draw_text_layout_box(fonts, FontFaceId::Cooper, line, 348.0f, 107.0f, 78.0f, 26.0f, 22.0f, TextAlign::Left, 1.0f, 1.0f, 1.0f, 1.0f, true);
        std::snprintf(line, sizeof(line), "%d", stone_bonus_);
        draw_text_layout_box(fonts, FontFaceId::Cooper, line, 458.0f, 107.0f, 60.0f, 26.0f, 22.0f, TextAlign::Left, 1.0f, 1.0f, 1.0f, 1.0f, true);

        draw_sprite(kExPieceBg, 299.0f, 194.0f, kXpBarW, kXpBarH, 1.0f);
        const float xp_ratio = std::clamp(static_cast<float>(show_xp_) / static_cast<float>(std::max(1, show_xp_max_)), 0.0f, 1.0f);
        draw_sprite(kExPiece, 299.0f, 194.0f, kXpBarW * xp_ratio, kXpBarH, 1.0f);
        draw_sprite(kExPanel, 295.0f, 190.0f, kXpPanelW, kXpPanelH, 1.0f);
        std::snprintf(line, sizeof(line), "%d/%d", show_xp_, show_xp_max_);
        draw_text_layout_box(fonts, FontFaceId::Cooper, line, 340.0f, 193.0f, 160.0f, 20.0f, 15.0f, TextAlign::Center, 1.0f, 1.0f, 1.0f, 1.0f, true);

        draw_sprite_centered(kWinStamp, is_win_ ? 275.0f : 525.0f, 410.0f, kWinStampW, kWinStampH, 1.0f);
        draw_sprite(report_pressed_ ? kReportDown : kReportUp, kReportX, kReportY, kReportW, kReportH, 1.0f);
    } else {
        const char* bonus_path = Param::language == 1 ? kStatsBonusKr : kStatsBonus;
        const char* life_path = Param::language == 1 ? kStatsLifeKr : kStatsLife;
        const char* kills_path = Param::language == 1 ? kStatsKillsKr : kStatsKills;
        const char* fg_path = is_win_
            ? (Param::language == 1 ? kFgWinKr : kFgWin)
            : (Param::language == 1 ? kFgLoseKr : kFgLose);
        draw_sprite(bonus_path, kLocalLabelX, kLocalBonusLabelY, kPieceLabelW, kPieceLabelH, 1.0f);
        draw_sprite(life_path, kLocalLabelX, kLocalLifeLabelY, kPieceLabelW, kPieceLabelH, 1.0f);
        draw_sprite(kills_path, kLocalLabelX, kLocalKillsLabelY, kPieceLabelW, kPieceLabelH, 1.0f);
        draw_sprite(kStatsXp, kLocalLabelX, kLocalXpLabelY, kPieceLabelW, kPieceLabelH, 1.0f);

        draw_sprite(kExPieceBg, kLocalXpFillX, kLocalXpFillY, kXpBarW, kXpBarH, 1.0f);
        const float xp_ratio = std::clamp(static_cast<float>(show_xp_) / static_cast<float>(std::max(1, show_xp_max_)), 0.0f, 1.0f);
        draw_sprite(kExPiece, kLocalXpFillX, kLocalXpFillY, kXpBarW * xp_ratio, kXpBarH, 1.0f);
        draw_sprite(kExPanel, kLocalXpPanelX, kLocalXpPanelY, kXpPanelW, kXpPanelH, 1.0f);

        std::snprintf(line, sizeof(line), "%s %d", level_word().c_str(), show_level_);
        draw_text_layout_box_with_offset(fonts, FontFaceId::Ants, line, kLocalLevelBoxX, kLocalLevelBoxY, kLocalLevelBoxW, kLocalLevelBoxH, 25.0f, TextAlign::Right, 1.0f, 1.0f, 1.0f, lv_alpha_, StageCompleteTextOffsetId::Level, true);
        std::snprintf(line, sizeof(line), "%d/%d", show_xp_, show_xp_max_);
        draw_text_layout_box_with_offset(fonts, FontFaceId::Cooper, line, kLocalXpBarTextX, kLocalXpBarTextY, kLocalXpBarTextW, kLocalXpBarTextH, 15.0f, TextAlign::Center, 1.0f, 1.0f, 1.0f, 1.0f, StageCompleteTextOffsetId::XpBar, true);

        std::snprintf(line, sizeof(line), "%d", gold_bonus_);
        draw_text_layout_box_with_offset(fonts, FontFaceId::Cooper, line, kLocalBonusCoinValueX, kLocalBonusCoinValueY, kLocalBonusCoinValueW, kLocalBonusValueH, 22.0f, TextAlign::Center, 1.0f, 1.0f, 1.0f, 1.0f, StageCompleteTextOffsetId::BonusGold, true);
        std::snprintf(line, sizeof(line), "%d", stone_bonus_);
        draw_text_layout_box_with_offset(fonts, FontFaceId::Cooper, line, kLocalBonusStoneValueX, kLocalBonusStoneValueY, kLocalBonusStoneValueW, kLocalBonusValueH, 22.0f, TextAlign::Center, 1.0f, 1.0f, 1.0f, 1.0f, StageCompleteTextOffsetId::BonusStone, true);
        std::snprintf(line, sizeof(line), "%d%%", life_bonus_);
        draw_text_layout_box_with_offset(fonts, FontFaceId::Cooper, line, kLocalRowValueX, kLocalLifeValueY, kLocalRowValueW, kLocalLifeValueH, 22.0f, TextAlign::Center, 1.0f, 1.0f, 1.0f, 1.0f, StageCompleteTextOffsetId::Life, true);
        std::snprintf(line, sizeof(line), "%d ", kill_bonus_);
        draw_text_layout_box_with_offset(fonts, FontFaceId::Cooper, line, kLocalRowValueX, kLocalKillsValueY, kLocalRowValueW, kLocalKillsValueH, 22.0f, TextAlign::Center, 1.0f, 1.0f, 1.0f, 1.0f, StageCompleteTextOffsetId::Kills, true);
        if (xp_skill_ > 0) {
            std::snprintf(line, sizeof(line), "%d +%d +%d", xp_base_, xp_achv_, xp_skill_);
        } else if (xp_achv_ > 0) {
            std::snprintf(line, sizeof(line), "%d +%d", xp_base_, xp_achv_);
        } else {
            std::snprintf(line, sizeof(line), "%d", xp_base_);
        }
        draw_text_layout_box_with_offset(fonts, FontFaceId::Cooper, line, kLocalRowValueX, kLocalXpGainY, kLocalRowValueW, kLocalXpGainH, 22.0f, TextAlign::Center, 1.0f, 1.0f, 1.0f, 1.0f, StageCompleteTextOffsetId::XpGain, true);

        draw_sprite_centered(fg_path, kLocalFgCenterX, kLocalFgCenterY, is_win_ ? kFgWinW : kFgLoseW, is_win_ ? kFgWinH : kFgLoseH, 1.0f);
    }

    if (count_done_) {
        const float alpha = std::abs(500.0f - static_cast<float>(AbstractGame::game_time_ms() % 1000ULL)) / 500.0f;
        const ScreenOffset& continue_offset = stage_complete_text_offset(StageCompleteTextOffsetId::Continue);
        draw_sprite_centered(kContinue, 400.0f + continue_offset.x, 452.5f + continue_offset.y, 410.0f, 47.0f, alpha);
    }
}

void StatsScene::update() {
    const int dt_ms = static_cast<int>(AbstractGame::last_span_ms());
    time_ms_ += dt_ms;
    xp_count_sound_cooldown_ms_ = std::max(0, xp_count_sound_cooldown_ms_ - dt_ms);

    if (result_sound_pending_) {
        AudioManager::instance().stop_music();
        AudioManager::instance().play_sound(is_win_ ? Sounds::GAME_COMP : Sounds::GAME_OVER);
        result_sound_pending_ = false;
    }

    if (time_ms_ > 500 && !rate_flag_) {
        // Android showed a rate prompt here; keep the timing gate but stub the platform action.
        rate_flag_ = true;
    }

    if (!count_done_ && (time_ms_ > 4000 || press_flag_)) {
        if (xp_pending_ > 0 && xp_count_sound_cooldown_ms_ <= 0) {
            AudioManager::instance().play_sound(Sounds::EXP_COUNT_SND);
            xp_count_sound_cooldown_ms_ = 180;
        }
        if (xp_pending_ > xp_step_) {
            xp_pending_ -= xp_step_;
            show_xp_ += xp_step_;
        } else {
            show_xp_ += xp_pending_;
            xp_pending_ = 0;
            count_done_ = true;
            xp_count_sound_cooldown_ms_ = 0;
        }
        while (show_xp_ >= show_xp_max_) {
            show_xp_ -= show_xp_max_;
            ++show_level_;
            AudioManager::instance().play_sound(Sounds::LEVEL_UP_SND);
            refresh_level_text();
            lv_alpha_ = 0.3f;
        }
    }

    if (lv_alpha_ < 1.0f) {
        lv_alpha_ += static_cast<float>(dt_ms) / 1000.0f;
        if (lv_alpha_ > 1.0f) {
            lv_alpha_ = 1.0f;
        }
    }
}

void StatsScene::reset() {
    time_ms_ = 0;
    press_flag_ = false;
    rate_flag_ = false;
    report_pressed_ = false;
    result_sound_pending_ = true;
    xp_count_sound_cooldown_ms_ = 0;
    apply_rewards();
    scene_x = 0.0f;
    scene_y = 0.0f;
    scene_scale = 1.0f;
    scene_alpha = 1.0f;
}

} // namespace defender
