#pragma once

#include <array>
#include <functional>
#include <string>

#include "scenes/scene.hpp"

namespace defender {

class OnlineDataScene : public Scene {
public:
    using TransitionRequest = std::function<void(int scene_id, int direction)>;
    using SetOnlineModeRequest = std::function<void(bool online_mode)>;

    OnlineDataScene(TransitionRequest transition_cb, SetOnlineModeRequest set_online_mode_cb);

    bool touch(const TouchEvent& event) override;
    void draw() override;
    void update() override;
    void reset() override;

private:
    enum class PressedButton {
        None,
        Single,
        Battle
    };

    static bool hit_test(const char* path,
                         float button_x,
                         float button_y,
                         float fallback_w,
                         float fallback_h,
                         float point_x,
                         float point_y);
    static void draw_sprite(const char* path,
                            float x,
                            float y,
                            float fallback_w,
                            float fallback_h,
                            float alpha = 1.0f);
    static void draw_bar_piece(const char* path,
                               float x,
                               float y,
                               float piece_w,
                               float piece_h,
                               float scale_x,
                               float alpha = 1.0f);

    TransitionRequest transition_cb_;
    SetOnlineModeRequest set_online_mode_cb_;
    PressedButton pressed_button_ = PressedButton::None;
    float shown_exp_ratio_ = 0.0f;
    float target_exp_ratio_ = 0.0f;
    int exp_max_ = 100;
    std::array<int, 8> achv_amounts_ = {};
    std::array<int, 8> achv_levels_ = {};
    std::array<bool, 8> achv_pressed_ = {};
    int achv_selected_ = 0;
    bool achv_popup_show_ = false;
    float achv_popup_alpha_ = 0.0f;
    int achv_popup_type_ = 0;
    int achv_popup_level_ = 0;
    int achv_popup_amount_ = 0;
    std::string pending_name_before_edit_;
    bool name_edit_pending_ = false;
};

} // namespace defender
