#pragma once

#include <memory>
#include <vector>

#include "engine/abstract_game.hpp"
#include "engine/touch_event.hpp"

namespace defender {

class Scene;
class MainGameScene;

class Game : public AbstractGame {
public:
    static constexpr int COVER = 0;
    static constexpr int MAIN_GAME = 1;
    static constexpr int RESEARCH = 2;
    static constexpr int STATS = 3;
    static constexpr int LOADING = 4;
    static constexpr int SHOP = 5;
    static constexpr int ONLINE_DATA = 6;

    static constexpr int TRANS_FROM_RIGHT = 0;
    static constexpr int TRANS_FROM_LEFT = 1;

    Game();
    ~Game();

    void initialize(int viewport_width, int viewport_height);
    void draw_frame() override;
    void update_frame() override;

    void touch(const TouchEvent& event);
    void tran_scene(int scene_id, int direction);
    void retry_act();
    void next_scene();
    bool is_main_game_over() const;

    int current_scene() const { return current_scene_; }
    int target_scene() const { return target_scene_; }

private:
    void refresh_audio_state();

    std::vector<std::unique_ptr<Scene>> scene_list_;
    MainGameScene* main_game_ = nullptr;
    MainGameScene* replay_game_ = nullptr;
    std::unique_ptr<MainGameScene> replay_scene_;

    bool transitioning_textures_ = false;
    bool audio_paused_ = false;
    int requested_music_resource_ = 0;
    bool requested_music_loop_ = false;
    int requested_music_restart_delay_ms_ = 0;
    int trans_direction_ = TRANS_FROM_RIGHT;
    int current_scene_ = LOADING;
    int target_scene_ = LOADING;
};

}
