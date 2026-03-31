#pragma once

#include <functional>
#include <string>

#include "scenes/scene.hpp"

namespace defender {

class StatsScene : public Scene {
public:
    using TransitionRequest = std::function<void(int scene_id, int direction)>;

    explicit StatsScene(TransitionRequest transition_cb);

    bool touch(const TouchEvent& event) override;
    void draw() override;
    void update() override;
    void reset() override;

private:
    void refresh_level_text();
    void apply_rewards();

    TransitionRequest transition_cb_;

    int battle_level_ = 1;
    int time_1_sec_ = 0;
    int time_2_sec_ = 0;
    bool is_win_ = false;
    bool online_mode_ = false;
    bool count_done_ = false;
    bool press_flag_ = false;
    bool rate_flag_ = false;
    bool report_pressed_ = false;
    bool result_sound_pending_ = false;

    std::string name_1_ = "player";
    std::string name_2_ = "rival";
    int time_ms_ = 0;
    int gold_bonus_ = 0;
    int stone_bonus_ = 0;
    int life_bonus_ = 0;
    int kill_bonus_ = 0;
    int xp_base_ = 0;
    int xp_achv_ = 0;
    int xp_skill_ = 0;
    int xp_pending_ = 0;
    int xp_step_ = 1;
    int xp_count_sound_cooldown_ms_ = 0;
    int show_level_ = 1;
    int show_xp_ = 0;
    int show_xp_max_ = 100;
    float lv_alpha_ = 1.0f;
};

} // namespace defender
