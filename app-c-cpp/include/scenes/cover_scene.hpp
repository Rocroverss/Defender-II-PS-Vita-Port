#pragma once

#include <functional>

#include "scenes/scene.hpp"

namespace defender {

class CoverScene : public Scene {
public:
    using TransitionRequest = std::function<void(int scene_id, int direction)>;

    explicit CoverScene(TransitionRequest transition_cb);

    bool touch(const TouchEvent& event) override;
    void draw() override;
    void update() override;
    void reset() override;

private:
    enum class PressedButton {
        None,
        Start,
        Honor,
        More
    };

    static bool hit_test(const char* path,
                         float button_x,
                         float button_y,
                         float fallback_w,
                         float fallback_h,
                         float point_x,
                         float point_y,
                         float pad_x = 0.0f,
                         float pad_y = 0.0f);
    static void draw_sprite(const char* path,
                            float x,
                            float y,
                            float fallback_w,
                            float fallback_h,
                            float alpha = 1.0f);
    static void draw_button(const char* up_path,
                            const char* down_path,
                            float x,
                            float y,
                            float fallback_w,
                            float fallback_h,
                            bool pressed,
                            float alpha = 1.0f);

    TransitionRequest transition_cb_;
    PressedButton pressed_button_ = PressedButton::None;
    float more_highlight_alpha_ = 1.0f;
    bool more_highlight_increasing_ = false;
};

} // namespace defender

