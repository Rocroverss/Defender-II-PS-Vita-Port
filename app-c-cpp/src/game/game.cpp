#include "game/game.hpp"

#include <cstdint>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <string>

#include "engine/bmp_scaler.hpp"
#include "engine/coordinate_mapper.hpp"
#include "engine/screen.hpp"
#include "engine/texture_cache.hpp"
#include "game/achv_mng.hpp"
#include "game/audio_manager.hpp"
#include "game/help_overlay.hpp"
#include "game/item_param.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "game/umeng_helper.hpp"
#include "scenes/cover_scene.hpp"
#include "scenes/loading_scene.hpp"
#include "scenes/main_game_scene.hpp"
#include "scenes/online_data_scene.hpp"
#include "scenes/research_scene.hpp"
#include "scenes/scene.hpp"
#include "scenes/shop_scene.hpp"
#include "scenes/stats_scene.hpp"

namespace defender {

namespace {

constexpr bool kTouchDebug = true;

const char* scene_name(int scene_id) {
    switch (scene_id) {
    case Game::COVER: return "COVER";
    case Game::MAIN_GAME: return "MAIN_GAME";
    case Game::RESEARCH: return "RESEARCH";
    case Game::STATS: return "STATS";
    case Game::LOADING: return "LOADING";
    case Game::SHOP: return "SHOP";
    case Game::ONLINE_DATA: return "ONLINE_DATA";
    default: return "UNKNOWN";
    }
}

const char* touch_action_name(TouchAction action) {
    switch (action) {
    case TouchAction::Down: return "Down";
    case TouchAction::Up: return "Up";
    case TouchAction::Move: return "Move";
    case TouchAction::None:
    default:
        return "None";
    }
}

void preload_texture_paths(std::initializer_list<const char*> paths) {
    auto& textures = TextureCache::instance();
    for (const char* path : paths) {
        if (path != nullptr && path[0] != '\0') {
            textures.get(path);
        }
    }
}

void preload_common_scene_textures() {
    preload_texture_paths({
        "assets/imgs_480_800/mode/load_line.png",
        "assets/imgs_480_800/mode/load_logo_light.png",
        "assets/imgs_480_800/mode/load_droidhen.png",
        "assets/imgs_480_800/mode/load_droidhen_light.png",
        "assets/imgs_480_800/mode/load_game.png",
        "assets/imgs_480_800/mode/load_game_light.png",
        "assets/imgs_480_800/mode/load_logo1.png",
        "assets/imgs_480_800/mode/load_logo2.png",
        "assets/imgs_480_800/mode/load_logo3.png",
        "assets/imgs_480_800/mode/load_logo4.png",
        "assets/imgs_480_800/mode/load_logo5.png",

        "assets/imgs_480_800/cover/cover_fixed.jpg",
        "assets/imgs_480_800/cover/zzz_button_music_on.png",
        "assets/imgs_480_800/cover/zzz_button_music_off.png",
        "assets/imgs_480_800/cover/button_sound_on.png",
        "assets/imgs_480_800/cover/button_sound_off.png",
        "assets/imgs_480_800/cover/cover_button_honor_up.png",
        "assets/imgs_480_800/cover/cover_button_honor_down.png",
        "assets/imgs_480_800/cover/cover_button_more_up.png",
        "assets/imgs_480_800/cover/cover_button_more_down.png",
        "assets/imgs_480_800/cover/cover_button_start_up.png",
        "assets/imgs_480_800/cover/cover_button_start_down.png",

        "assets/imgs_480_800/onlinedata/z_online_data_bg.jpg",
        "assets/imgs_480_800/onlinedata/z_online_data_bg_kr.jpg",
        "assets/imgs_480_800/onlinedata/z_button_start_up.png",
        "assets/imgs_480_800/onlinedata/z_button_start_down.png",
        "assets/imgs_480_800/onlinedata/button_start_up.png",
        "assets/imgs_480_800/onlinedata/button_start_down.png",
        "assets/imgs_480_800/onlinedata/button_start_up_kr.png",
        "assets/imgs_480_800/onlinedata/button_start_down_kr.png",
        "assets/imgs_480_800/shop/off_flag.png",
        "assets/imgs_480_800/onlinedata/ex_panel.png",
        "assets/imgs_480_800/onlinedata/ex_piece_bg.png",
        "assets/imgs_480_800/onlinedata/ex_piece.png",
        "assets/imgs_480_800/onlinedata/achieve_bg_panel.png",
        "assets/imgs_480_800/achieve/achieve_panel_bg.png",
        "assets/imgs_480_800/achieve/achieve_piece_bg.png",
        "assets/imgs_480_800/achieve/achieve_piece_frame.png",
        "assets/imgs_480_800/achieve/achieve_piece.png",
        "assets/imgs_480_800/achieve/achieve_logo_cast_0.png",

        "assets/imgs_480_800/research/research_bg_fixed.jpg",
        "assets/imgs_480_800/research/research_button_continue_up.png",
        "assets/imgs_480_800/research/research_button_continue_down.png",
        "assets/imgs_480_800/research/research_button_continue_up_kr.png",
        "assets/imgs_480_800/research/research_button_continue_down_kr.png",
        "assets/imgs_480_800/research/research_button_defender_up.png",
        "assets/imgs_480_800/research/research_button_defender_down.png",
        "assets/imgs_480_800/research/research_button_magic_up.png",
        "assets/imgs_480_800/research/research_button_magic_down.png",
        "assets/imgs_480_800/research/research_button_wall_up.png",
        "assets/imgs_480_800/research/research_button_wall_down.png",
        "assets/imgs_480_800/research/research_button_bow_up.png",
        "assets/imgs_480_800/research/research_button_bow_down.png",
        "assets/imgs_480_800/research/equip_cover_bg_fixed.png",
        "assets/imgs_480_800/research/research_defender_line.png",
        "assets/imgs_480_800/research/research_magic_line.png",
        "assets/imgs_480_800/research/research_wall_line.png",
        "assets/imgs_480_800/research/button_upgrade_up.png",
        "assets/imgs_480_800/research/button_upgrade_down.png",
        "assets/imgs_480_800/research/button_upgrade_up_kr.png",
        "assets/imgs_480_800/research/button_upgrade_down_kr.png",
        "assets/imgs_480_800/research/research_button_buy_up.png",
        "assets/imgs_480_800/research/research_button_buy_down.png",
        "assets/imgs_480_800/research/research_button_buy_up_kr.png",
        "assets/imgs_480_800/research/research_button_buy_down_kr.png",
        "assets/imgs_480_800/research/research_button_equip_up.png",
        "assets/imgs_480_800/research/research_button_equip_down.png",
        "assets/imgs_480_800/research/research_button_equip_up_kr.png",
        "assets/imgs_480_800/research/research_button_equip_down_kr.png",
        "assets/imgs_480_800/game/coin.png",
        "assets/imgs_480_800/game/mana_stone.png",
        "assets/imgs_480_800/game/z_number_list.png",
        "assets/imgs_480_800/game/z_number_list_level.png",

        "assets/imgs_480_800/shop/shop_bg_fixed.jpg",
        "assets/imgs_480_800/shop/shop_bt1_up.png",
        "assets/imgs_480_800/shop/shop_bt1_down.png",
        "assets/imgs_480_800/shop/shop_bt2_up.png",
        "assets/imgs_480_800/shop/shop_bt2_down.png",
        "assets/imgs_480_800/shop/shop_bt3_up.png",
        "assets/imgs_480_800/shop/shop_bt3_down.png",
        "assets/imgs_480_800/shop/shop_bt4_up.png",
        "assets/imgs_480_800/shop/shop_bt4_down.png",
        "assets/imgs_480_800/shop/shop_bt5_up.png",
        "assets/imgs_480_800/shop/shop_bt5_down.png",
        "assets/imgs_480_800/shop/shop_bt6_up.png",
        "assets/imgs_480_800/shop/shop_bt6_down.png",
        "assets/imgs_480_800/shop/shop_btfree_up.png",
        "assets/imgs_480_800/shop/shop_btfree_down.png",

        "assets/imgs_480_800/stats/stats_local_bg.jpg",
        "assets/imgs_480_800/stats/stats_online_bg.jpg",
        "assets/imgs_480_800/stats/stats_online_bg_kr.jpg",
        "assets/imgs_480_800/stats/stats_win_fg.png",
        "assets/imgs_480_800/stats/stats_win_fg_kr.png",
        "assets/imgs_480_800/stats/stats_failed_fg.png",
        "assets/imgs_480_800/stats/stats_failed_fg_kr.png",
        "assets/imgs_480_800/stats/stats_win_stamp.png",
        "assets/imgs_480_800/stats/button_report_up.png",
        "assets/imgs_480_800/stats/button_report_down.png",
        "assets/imgs_480_800/stats/stats_piece_bonus.png",
        "assets/imgs_480_800/stats/stats_piece_bonus_kr.png",
        "assets/imgs_480_800/stats/stats_piece_life.png",
        "assets/imgs_480_800/stats/stats_piece_life_kr.png",
        "assets/imgs_480_800/stats/stats_piece_kills.png",
        "assets/imgs_480_800/stats/stats_piece_kills_kr.png",
        "assets/imgs_480_800/stats/stats_piece_xp.png",
    });
}

void preload_common_audio(AudioManager& audio) {
    audio.preload_sound(Sounds::COVER_BGM_SND);
    audio.preload_sound(Sounds::STAGE_BGM);
    audio.preload_sound(Sounds::BOSS_BGM);
    audio.preload_sound(Sounds::BUTTON_CLICK_SND);
    audio.preload_sound(Sounds::BUTTON_UPGREADE);
    audio.preload_sound(Sounds::GAME_COMP);
    audio.preload_sound(Sounds::GAME_OVER);
    audio.preload_sound(Sounds::EXP_COUNT_SND);
    audio.preload_sound(Sounds::ARROW_SHOT_SND);
    audio.preload_sound(Sounds::BEHIT_SND);
    audio.preload_sound(Sounds::MAGIC_FIRE_1);
    audio.preload_sound(Sounds::MAGIC_ICE_1);
    audio.preload_sound(Sounds::LIGHTNING_1_SND);
    audio.preload_sound(Sounds::WARNING_SND);
}

} // namespace

Game::Game() {
    Save::init();
    UMengHelper::init();

    Param::is_show_ad = Save::load_data(Save::HIDE_AD, Save::GLOBAL_DATA) == 0;
    Param::music_flag = Save::load_data(Save::MUSIC_FLAG, Save::GLOBAL_DATA) == 0;
    Param::sound_effect_flag = Save::load_data(Save::SOUND_FLAG, Save::GLOBAL_DATA) == 0;

    Param::gold = Save::load_data(Save::GOLD);
    Param::stone = Save::load_data(Save::STONE);
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

    ItemParam::load_level();

    scene_list_.resize(7);

    auto transition_cb = [this](int scene_id, int direction) {
        tran_scene(scene_id, direction);
    };
    auto set_online_mode_cb = [this](bool online_mode) {
        Param::is_online_mode = online_mode;
        if (main_game_ != nullptr) {
            main_game_->set_online_mode(online_mode);
        }
    };

    scene_list_[COVER] = std::make_unique<CoverScene>(transition_cb);

    auto main_game = std::make_unique<MainGameScene>(transition_cb);
    main_game->set_online_mode(false);
    main_game_ = main_game.get();
    scene_list_[MAIN_GAME] = std::move(main_game);

    replay_scene_ = std::make_unique<MainGameScene>(transition_cb);
    replay_scene_->set_replay_mode(true);
    replay_scene_->set_online_mode(true);
    replay_game_ = replay_scene_.get();

    scene_list_[RESEARCH] = std::make_unique<ResearchScene>(transition_cb);
    scene_list_[STATS] = std::make_unique<StatsScene>(transition_cb);
    scene_list_[LOADING] = std::make_unique<LoadingScene>(transition_cb);
    scene_list_[SHOP] = std::make_unique<ShopScene>(transition_cb);
    scene_list_[ONLINE_DATA] = std::make_unique<OnlineDataScene>(transition_cb, set_online_mode_cb);

    AchvMng::init();
    HelpOverlay::init();
}

Game::~Game() {
    AudioManager::instance().shutdown();
}

void Game::initialize(int viewport_width, int viewport_height) {
    UMengHelper::on_resume();
    AudioManager::instance().initialize();

    auto& screen = Screen::current();
    screen.set_bounds(viewport_width, viewport_height);

    auto& mapper = CoordinateMapper::instance();
    mapper.set_designed(static_cast<float>(Param::DESIGNED_SCREEN_WIDTH), static_cast<float>(Param::DESIGNED_SCREEN_HEIGHT));

    auto& bmp = BmpScaler::instance();
    bmp.set_bitmap_original(static_cast<float>(Param::DESIGNED_SCREEN_WIDTH), static_cast<float>(Param::DESIGNED_SCREEN_HEIGHT));

    Scene::scene_init();
    preload_common_scene_textures();
    if (main_game_ != nullptr) {
        main_game_->preload_assets();
    }
    preload_common_audio(AudioManager::instance());
    current_scene_ = LOADING;
    target_scene_ = LOADING;
    transitioning_textures_ = false;
    scene_list_[LOADING]->reset();
}

void Game::draw_frame() {
    update_frame();
    scene_list_[current_scene_]->scene_draw();

    if (current_scene_ != target_scene_ && !transitioning_textures_) {
        scene_list_[target_scene_]->scene_draw();
    }
}

void Game::update_frame() {
    AbstractGame::update_frame();
    refresh_audio_state();

    if (target_scene_ == current_scene_) {
        scene_list_[current_scene_]->update();
    }

    if (target_scene_ != current_scene_ && !transitioning_textures_) {
        scene_list_[target_scene_]->update();

        if (trans_direction_ == TRANS_FROM_RIGHT) {
            float temp = static_cast<float>(last_span_ms()) * scene_list_[target_scene_]->scene_x / 80.0f;
            if (temp < 20.0f) {
                temp = 20.0f;
            }
            scene_list_[target_scene_]->scene_x -= temp;
            scene_list_[current_scene_]->scene_x -= temp;
            if (scene_list_[target_scene_]->scene_x <= 0.0f) {
                scene_list_[target_scene_]->scene_x = 0.0f;
                current_scene_ = target_scene_;
            }
        } else if (trans_direction_ == TRANS_FROM_LEFT) {
            float temp = (static_cast<float>(-static_cast<int64_t>(last_span_ms())) * scene_list_[target_scene_]->scene_x) / 80.0f;
            if (temp < 20.0f) {
                temp = 20.0f;
            }
            scene_list_[target_scene_]->scene_x += temp;
            scene_list_[current_scene_]->scene_x += temp;
            if (scene_list_[target_scene_]->scene_x >= 0.0f) {
                scene_list_[target_scene_]->scene_x = 0.0f;
                current_scene_ = target_scene_;
            }
        }
    }

    AchvMng::update();
    AudioManager::instance().update();
}

void Game::touch(const TouchEvent& event) {
    if (kTouchDebug && event.action == TouchAction::Down) {
        std::printf(
            "[touch] action=%s scene=%s(%d) target=%s(%d) x1=%.1f y1=%.1f x2=%.1f y2=%.1f pointers=%d\n",
            touch_action_name(event.action),
            scene_name(current_scene_),
            current_scene_,
            scene_name(target_scene_),
            target_scene_,
            event.x1,
            event.y1,
            event.x2,
            event.y2,
            event.pointer_count
        );
        std::fflush(stdout);
    }
    if (current_scene_ == target_scene_) {
        scene_list_[current_scene_]->touch(event);
    }
}

void Game::tran_scene(int scene_id, int direction) {
    if (scene_id < 0 || scene_id >= static_cast<int>(scene_list_.size())) {
        return;
    }

    if (direction == TRANS_FROM_RIGHT) {
        scene_list_[scene_id]->scene_x = Scene::screen_width;
    } else if (direction == TRANS_FROM_LEFT) {
        scene_list_[scene_id]->scene_x = -Scene::screen_width;
    }

    resume();

    switch (scene_id) {
    case COVER:
        scene_list_[COVER]->reset();
        break;
    case MAIN_GAME:
        replay_game_->reset();
        main_game_->reset();
        transitioning_textures_ = false;
        break;
    case RESEARCH:
        scene_list_[RESEARCH]->reset();
        break;
    case STATS:
        scene_list_[STATS]->reset();
        break;
    case LOADING:
        scene_list_[LOADING]->reset();
        break;
    case SHOP:
        scene_list_[SHOP]->reset();
        break;
    case ONLINE_DATA:
        scene_list_[ONLINE_DATA]->reset();
        break;
    default:
        break;
    }

    trans_direction_ = direction;
    target_scene_ = scene_id;
}

void Game::retry_act() {
    Save::pause_save_data();
    resume();
    if (Param::is_online_mode) {
        ++Param::lose;
        Save::save_data(Save::LOSE, Param::lose);
        tran_scene(ONLINE_DATA, TRANS_FROM_LEFT);
    } else {
        tran_scene(MAIN_GAME, TRANS_FROM_LEFT);
    }
}

void Game::next_scene() {
    const int next = (target_scene_ + 1) % static_cast<int>(scene_list_.size());
    tran_scene(next, TRANS_FROM_RIGHT);
}

bool Game::is_main_game_over() const {
    return main_game_ != nullptr && main_game_->is_game_over();
}

void Game::refresh_audio_state() {
    auto& audio = AudioManager::instance();
    audio.set_music_enabled(Param::music_flag);
    audio.set_sound_enabled(Param::sound_effect_flag);

    if (!Param::music_flag) {
        audio.stop_music();
        requested_music_resource_ = 0;
        requested_music_loop_ = false;
        requested_music_restart_delay_ms_ = 0;
        return;
    }

    const bool paused = AbstractGame::is_paused();
    if (paused != audio_paused_) {
        audio.set_paused(paused);
        audio_paused_ = paused;
    }
    if (paused) {
        return;
    }

    const SoundType* desired_music = nullptr;
    bool desired_loop = false;
    int desired_restart_delay_ms = 0;
    // Delay music changes until the scene transition has actually landed.
    // Switching early caused abrupt stop/start stutter during slide transitions.
    const int scene_for_audio = current_scene_;
    switch (scene_for_audio) {
    case COVER:
    case RESEARCH:
    case SHOP:
    case ONLINE_DATA:
        desired_music = &Sounds::COVER_BGM_SND;
        desired_loop = true;
        desired_restart_delay_ms = Sounds::loop_cycle_duration_ms(desired_music->resource_id);
        break;
    case MAIN_GAME:
        if (main_game_ != nullptr) {
            if (!main_game_->is_game_over() &&
                !Param::is_online_mode &&
                (Param::stage % 10) == 0 &&
                main_game_->is_boss_active()) {
                desired_music = &Sounds::BOSS_BGM;
                desired_loop = true;
                desired_restart_delay_ms = Sounds::loop_cycle_duration_ms(desired_music->resource_id);
            } else if (!main_game_->is_game_over()) {
                desired_music = &Sounds::STAGE_BGM;
                desired_loop = true;
                desired_restart_delay_ms = Sounds::loop_cycle_duration_ms(desired_music->resource_id);
            }
        }
        break;
    case STATS:
        desired_music = nullptr;
        break;
    case LOADING:
    default:
        desired_music = nullptr;
        break;
    }

    if (desired_music == nullptr) {
        audio.stop_music();
        requested_music_resource_ = 0;
        requested_music_loop_ = false;
        requested_music_restart_delay_ms_ = 0;
        return;
    }

    const bool music_changed =
        requested_music_resource_ != desired_music->resource_id ||
        requested_music_loop_ != desired_loop ||
        requested_music_restart_delay_ms_ != desired_restart_delay_ms;

    if (music_changed) {
        audio.play_music(*desired_music, desired_loop, desired_restart_delay_ms);
        requested_music_resource_ = desired_music->resource_id;
        requested_music_loop_ = desired_loop;
        requested_music_restart_delay_ms_ = desired_restart_delay_ms;
        return;
    }

    if (desired_loop && audio.current_music_resource() != desired_music->resource_id) {
        audio.play_music(*desired_music, desired_loop, desired_restart_delay_ms);
    }
}

}
