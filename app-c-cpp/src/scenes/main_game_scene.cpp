#include "scenes/main_game_scene.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <string>

#include "engine/abstract_game.hpp"
#include "engine/game_texture_specs.hpp"
#include "engine/render_utils.hpp"
#include "engine/sprite_atlas_cache.hpp"
#include "engine/texture_cache.hpp"
#include "game/achv_data.hpp"
#include "game/audio_manager.hpp"
#include "game/help_overlay.hpp"
#include "game/item_param.hpp"
#include "game/param.hpp"
#include "game/skill_data.hpp"
#include "game/sounds.hpp"
#include "game/umeng_helper.hpp"

namespace defender {

namespace {

constexpr int kGameOverTipDelayMs = 4000;
constexpr int kGameOverAutoTransitionMs = 4500;
constexpr int kGameFinishTransitionMs = 1000;
constexpr float kTopHudTouchSafeHeight = 64.0f;
constexpr float kBottomHudTouchSafeHeight = 96.0f;

const TextureHandle& game_texture(const std::string& file_name) {
    return TextureCache::instance().get("assets/imgs_480_800/game/" + file_name);
}

int item_level(const MainGameScene* scene, int type) {
    return scene->is_rep() ? ItemParam::get_level(type, false) : ItemParam::get_level(type);
}

const char* river_bg_for_level(int river_level) {
    switch (std::clamp(river_level, 1, 3)) {
    case 1: return "river_lv1a.jpg";
    case 2: return "river_lv2a.jpg";
    default: return "river_lv3a.jpg";
    }
}

const char* river_overlay_for_level(int river_level) {
    switch (std::clamp(river_level, 1, 3)) {
    case 1: return "river_lv1.jpg";
    case 2: return "river_lv2.jpg";
    default: return "river_lv3.jpg";
    }
}

void draw_texture_cover_screen(const TextureHandle& tex, float alpha = 1.0f) {
    if (!tex.valid || tex.id == 0 || tex.width <= 0 || tex.height <= 0) {
        return;
    }
    draw_textured_quad_cover(
        tex.id,
        0.0f,
        0.0f,
        Scene::screen_width,
        Scene::screen_height,
        static_cast<float>(tex.width),
        static_cast<float>(tex.height),
        alpha
    );
}

void draw_texture_fill_screen(const TextureHandle& tex, float alpha = 1.0f) {
    if (!tex.valid || tex.id == 0 || tex.width <= 0 || tex.height <= 0) {
        return;
    }
    draw_textured_quad(tex.id, 0.0f, 0.0f, Scene::screen_width, Scene::screen_height, alpha);
}

}

MainGameScene::MainGameScene(TransitionRequest transition_cb) : transition_cb_(transition_cb) {
    reset();
}

void MainGameScene::set_replay_mode(bool replay_mode) {
    is_replay_mode_ = replay_mode;
}

void MainGameScene::set_online_mode(bool online_mode) {
    is_online_mode_ = online_mode;
}

bool MainGameScene::need_rec() const {
    return is_online_mode_ && !is_replay_mode_;
}

bool MainGameScene::is_rep() const {
    return is_online_mode_ && is_replay_mode_;
}

void MainGameScene::random_init() {
    if (Param::is_online_mode) {
        form_rng_.seed(static_cast<uint32_t>(Param::random_seed1));
        boss1_rng_.seed(static_cast<uint32_t>(Param::random_seed2));
        boss2_rng_.seed(static_cast<uint32_t>(Param::random_seed3));
        return;
    }

    std::random_device rd;
    form_rng_.seed(rd());
    boss1_rng_.seed(rd());
    boss2_rng_.seed(rd());
}

void MainGameScene::reset() {
    preload_assets();

    random_init();
    AbstractGame::reset_game_time();
    if (!is_online_mode_) {
        UMengHelper::start_level(Param::stage);
    }

    is_shotting_ = false;
    is_spelling_ = false;
    is_game_over_ = false;
    is_game_finish_ = false;
    is_playsound_ = false;
    show_small_window_ = false;
    gameover_time_ms_ = 0;
    gameover_bg_alpha_ = 0.0f;
    gameover_word_alpha_ = 0.0f;
    tip_alpha_ = 0.0f;
    gameover_transition_sent_ = false;
    shot_x_ = 0.0f;
    shot_y_ = 0.0f;
    selected_magic_slot_ = -1;
    magic_show_range_ = false;
    magic_using_.clear();
    magic_recycle_.clear();
    boss_warning_sound_played_ = false;
    next_monster_uid_ = 1;

    Param::kills = 0;
    Param::add_mana_data = 0;
    Param::life_percent = 100;
    Param::time = 0;
    Param::rep_time = 0;
    Param::single_battle_time = 0;
    Param::extra_gold = AchvData::get_reward(0, AchvData::get_level(0, Param::cost_coin));
    Param::extra_mana = AchvData::get_reward(1, AchvData::get_level(1, Param::cost_stone));
    Param::extra_dmg = AchvData::get_reward(2, AchvData::get_level(2, Param::total_kills));
    Param::extra_fire = AchvData::get_reward(5, AchvData::get_level(5, Param::cast_fire));
    Param::extra_ice = AchvData::get_reward(6, AchvData::get_level(6, Param::cast_ice));
    Param::extra_light = AchvData::get_reward(7, AchvData::get_level(7, Param::cast_light));
    Param::extra_local_xp = AchvData::get_reward(3, AchvData::get_level(3, Param::stage));
    Param::extra_battle_xp = AchvData::get_reward(4, AchvData::get_level(4, Param::win));
    Param::spell_data.fill(0);

    scene_x = 0.0f;
    scene_y = 0.0f;
    scene_scale = 1.0f;
    scene_alpha = 1.0f;

    arrows_.clear();
    monsters_.clear();
    enemy_projectiles_.clear();
    effects_.clear();

    bow_x_ = get_x(10.0f);
    bow_y_ = get_y(240.0f);
    bow_angle_deg_ = 0.0f;
    target_angle_deg_ = 0.0f;

    wall_x_ = get_x(150.0f);
    wall_max_hp_ = SkillData::get_value(SkillData::WALL);
    wall_hp_ = wall_max_hp_;
    wall_max_mana_ = SkillData::get_value(SkillData::MANA_BASIC);
    wall_mana_ = wall_max_mana_;
    mana_regen_per_sec_ = static_cast<float>(wall_max_mana_) / 100.0f;
    add_mana_bonus_ = 0;
    add_mana_cd_left_ms_ = 0;
    bow_cd_left_ms_ = 0;

    init_bow_state();
    init_magic_state();
    init_wall_defenders();
    configure_stage();
}

void MainGameScene::preload_assets() {
    warmup_textures();
}

void MainGameScene::warmup_textures() {
    if (textures_warm_) {
        return;
    }
    textures_warm_ = true;

    game_texture("bg_lv0.jpg");
    game_texture("bg_lv1.jpg");
    game_texture("bg_lv2.jpg");
    game_texture("bg_lv3.jpg");
    game_texture("river_lv1a.jpg");
    game_texture("river_lv1.jpg");
    game_texture("river_lv2a.jpg");
    game_texture("river_lv2.jpg");
    game_texture("river_lv3a.jpg");
    game_texture("river_lv3.jpg");
    game_texture("bow_normal.png");
    game_texture("arrow_normal.png");
    game_texture("monster_logo.png");
    game_texture("normal_lv1.png");
    game_texture("normal_lv2.png");
    game_texture("normal_lv3.png");
    game_texture("normal_001.png");
    game_texture("normal_002.png");
    game_texture("normal_003.png");
    game_texture("normal_004.png");
    game_texture("normal_005.png");
    game_texture("normal_006.png");
    game_texture("normal_007.png");
    game_texture("normal_008.png");
    game_texture("normal_009.png");
    game_texture("normal_010.png");
    game_texture("wall_broken_1.png");
    game_texture("wall_broken_2.png");
    game_texture("sword_logo.png");
    game_texture("button_addmana.png");
    game_texture("magic_button_fire.png");
    game_texture("magic_button_ice.png");
    game_texture("magic_button_elect.png");
    game_texture("magic_button_lock.png");
    game_texture("magic_button_lowmana_bg.png");
    game_texture("magic_button_lowmana_word.png");
    game_texture("magic_button_flash.png");
    game_texture("magic_cd.png");
    game_texture("warning.png");
    game_texture("normal_lv1.png");
    game_texture("ring_normal_lv3.png");
    game_texture("ball_lv3_001.png");
    game_texture("gameover_bg_fixed.jpg");
    game_texture("gameover_word.png");
    game_texture("gameover_tips.png");
    TextureCache::instance().get("assets/imgs_480_800/always/critical_hit.png");
    SpriteAtlasCache::instance().warmup_default();
}

bool MainGameScene::touch(const TouchEvent& event) {
    if (Param::stage == 1 && HelpOverlay::touch(event, event.x1, event.y1)) {
        return true;
    }

    if (is_game_over_) {
        if (!gameover_transition_sent_ && gameover_time_ms_ > kGameOverTipDelayMs && event.action == TouchAction::Down) {
            gameover_time_ms_ = 400000;
            Param::is_win = false;
            gameover_transition_sent_ = true;
            if (transition_cb_) {
                transition_cb_(Param::SCENE_STATS, 0);
            }
        }
        return true;
    }

    if (panel_touch(event)) {
        return true;
    }

    const float hud_top = screen_height - get_y(kBottomHudTouchSafeHeight);
    const float hud_status_bottom = get_y(kTopHudTouchSafeHeight);
    if (is_spelling_) {
        return true;
    }
    switch (event.action) {
    case TouchAction::Down:
        if (event.y1 > hud_status_bottom && event.y1 < hud_top) {
            is_shotting_ = true;
            shot_x_ = event.x1;
            shot_y_ = event.y1;
        }
        break;
    case TouchAction::Move:
        if (is_shotting_) {
            shot_x_ = event.x1;
            shot_y_ = event.y1;
        }
        break;
    case TouchAction::Up:
        is_shotting_ = false;
        break;
    default:
        break;
    }

    return true;
}

void MainGameScene::draw() {
    const auto& bg = game_texture("bg_lv0.jpg");
    if (bg.valid) {
        draw_game_texture_quad(
            bg,
            "bg_lv0.jpg",
            0.0f,
            0.0f,
            screen_width,
            screen_height,
            1.0f
        );
    } else {
        draw_quad(0.0f, 0.0f, screen_width, screen_height, 0.08f, 0.11f, 0.16f, 1.0f);
    }

    const int river_level = item_level(this, ItemParam::RIVER);
    if (river_level > 0) {
        const std::string river_name = river_bg_for_level(river_level);
        const std::string river_overlay_name = river_overlay_for_level(river_level);
        const auto& river = game_texture(river_name);
        const float river_x = get_x(141.0f);
        const float river_w = get_x(
            static_cast<float>(game_texture_width_px(river, river_name)));
        if (river.valid) {
            draw_game_texture_quad(river, river_name, river_x, 0.0f, river_w, screen_height, 1.0f);
        }
        const auto& river_overlay = game_texture(river_overlay_name);
        if (river_overlay.valid) {
            const float t = static_cast<float>(AbstractGame::game_time_ms() % 2000ULL);
            const float cover_alpha = std::abs(1000.0f - t) / 1000.0f;
            draw_game_texture_quad(
                river_overlay,
                river_overlay_name,
                river_x,
                0.0f,
                river_w,
                screen_height,
                cover_alpha
            );
        }
    }

    const float hp_ratio = static_cast<float>(wall_hp_) / std::max(1.0f, static_cast<float>(wall_max_hp_));
    const float mana_ratio = static_cast<float>(wall_mana_) / std::max(1.0f, static_cast<float>(wall_max_mana_));
    draw_wall_layer(hp_ratio);
    draw_wall_defender_layer();
    draw_arrow_layer();
    draw_bow_layer();
    draw_auto_turrets();
    draw_effect_layer();

    const int boss_warning_offset_ms = 57000 + ((Param::stage * 1000) / 15);
    if ((Param::stage % 10) == 0 &&
        !is_game_over_ &&
        !Param::is_online_mode &&
        AbstractGame::game_time_ms() > static_cast<uint64_t>(boss_warning_offset_ms) &&
        AbstractGame::game_time_ms() < static_cast<uint64_t>(boss_warning_offset_ms + 2000)) {
        const auto& warning_tex = game_texture("warning.png");
        if (warning_tex.valid) {
            const float pulse = std::abs(static_cast<float>((AbstractGame::game_time_ms() % 1000ULL)) - 500.0f) / 500.0f;
            const float warning_w = get_x(static_cast<float>(game_texture_width_px(warning_tex, "warning.png")));
            const float warning_h = get_y(static_cast<float>(game_texture_height_px(warning_tex, "warning.png")));
            draw_game_texture_quad_tinted(
                warning_tex,
                "warning.png",
                get_x(400.0f) - (warning_w * 0.5f),
                get_y(240.0f) - (warning_h * 0.5f),
                warning_w,
                warning_h,
                1.0f,
                pulse,
                pulse,
                1.0f
            );
        }
    }

    draw_panel_layer(hp_ratio, mana_ratio);

    if (is_game_over_) {
        const auto& gameover_bg = game_texture("gameover_bg_fixed.jpg");
        if (gameover_bg.valid) {
            draw_texture_fill_screen(gameover_bg, gameover_bg_alpha_);
        } else {
            draw_quad(0.0f, 0.0f, screen_width, screen_height, 0.0f, 0.0f, 0.0f, gameover_bg_alpha_);
        }

        const auto& gameover_word = game_texture("gameover_word.png");
        if (gameover_word.valid) {
            draw_game_texture_quad(
                gameover_word,
                "gameover_word.png",
                get_x(240.0f),
                get_y(180.0f),
                get_x(320.0f),
                get_y(120.0f),
                gameover_word_alpha_
            );
        } else {
            const float wr = is_game_finish_ ? 0.15f : 0.75f;
            const float wg = is_game_finish_ ? 0.72f : 0.1f;
            const float wb = is_game_finish_ ? 0.20f : 0.1f;
            draw_quad(get_x(240.0f), get_y(180.0f), get_x(320.0f), get_y(120.0f), wr, wg, wb, gameover_word_alpha_);
        }

        if (gameover_time_ms_ > kGameOverTipDelayMs && !is_rep()) {
            const auto& gameover_tip = game_texture("gameover_tips.png");
            if (gameover_tip.valid) {
                draw_game_texture_quad(
                    gameover_tip,
                    "gameover_tips.png",
                    get_x(270.0f),
                    get_y(120.0f),
                    get_x(260.0f),
                    get_y(48.0f),
                    tip_alpha_
                );
            } else {
                draw_quad(get_x(270.0f), get_y(120.0f), get_x(260.0f), get_y(48.0f), 0.92f, 0.92f, 0.92f, tip_alpha_);
            }
        }
    }
}

void MainGameScene::update() {
    Param::single_battle_time = static_cast<int>(AbstractGame::game_time_ms());
    Param::time = Param::single_battle_time;
    HelpOverlay::update(true);

    const float dt_ms = static_cast<float>(AbstractGame::last_span_ms());
    if (dt_ms <= 0.0f) {
        return;
    }

    const bool help_showing = (Param::stage == 1) && HelpOverlay::is_show();

    if (!Param::is_online_mode && (Param::stage % 10) == 0) {
        const int warning_start_ms = 57000 + ((Param::stage * 1000) / 15);
        const int boss_bgm_start_ms = 60000 + ((Param::stage * 1000) / 15);
        const uint64_t now_ms = AbstractGame::game_time_ms();
        if (now_ms > static_cast<uint64_t>(warning_start_ms) &&
            now_ms < static_cast<uint64_t>(boss_bgm_start_ms) &&
            !boss_warning_sound_played_ &&
            !is_game_over_) {
            AudioManager::instance().play_sound(Sounds::WARNING_SND);
            boss_warning_sound_played_ = true;
        }
        if (now_ms <= static_cast<uint64_t>(warning_start_ms)) {
            boss_warning_sound_played_ = false;
        }
    }

    if (is_game_over_) {
        is_shotting_ = false;
    } else {
        update_wall_state(dt_ms);

        if (!help_showing && is_shotting_) {
            shoot_volley(shot_x_, shot_y_);
        }

        if (!help_showing) {
            update_monsters(dt_ms);
            update_wave();
            update_enemy_projectiles(dt_ms);
            update_arrows(dt_ms);
            magic_update_java(dt_ms);
            update_effects(dt_ms);
            update_mission_state();
            update_wall_defenders(dt_ms);
        }

        if (wall_hp_ <= 0) {
            is_game_over_ = true;
            gameover_time_ms_ = 0;
            if (!is_rep()) {
                AudioManager::instance().stop_music();
                AudioManager::instance().play_sound(Sounds::GAME_OVER);
            }
        }

        if (is_game_over_) {
            is_shotting_ = false;
            is_spelling_ = false;
            arrows_.clear();
            enemy_projectiles_.clear();
            effects_.clear();
            magic_using_.clear();
            gameover_time_ms_ = 0;
        }
    }

    if (is_game_over_) {
        gameover_time_ms_ += static_cast<int>(dt_ms);
        gameover_bg_alpha_ = std::min(0.35f, static_cast<float>(gameover_time_ms_) / 2600.0f);
        gameover_word_alpha_ = std::min(1.0f, static_cast<float>(gameover_time_ms_ + 350) / 900.0f);
        if (gameover_time_ms_ > kGameOverTipDelayMs) {
            tip_alpha_ = (500.0f - static_cast<float>(std::abs((gameover_time_ms_ % 1000) - 500))) / 500.0f;
        }
    }

    if (is_game_finish_ && monsters_.empty()) {
        gameover_time_ms_ += static_cast<int>(dt_ms);
    }

    if (is_game_over_ && !is_rep() && !gameover_transition_sent_ && gameover_time_ms_ > kGameOverAutoTransitionMs) {
        Param::is_win = false;
        gameover_transition_sent_ = true;
        if (transition_cb_) {
            transition_cb_(Param::SCENE_STATS, 0);
        }
    }

    if (is_game_finish_ &&
        monsters_.empty() &&
        !gameover_transition_sent_ &&
        gameover_time_ms_ > kGameFinishTransitionMs) {
        Param::is_win = true;
        gameover_transition_sent_ = true;
        if (transition_cb_) {
            transition_cb_(Param::SCENE_STATS, 0);
        }
        is_game_finish_ = false;
    }
}

}
