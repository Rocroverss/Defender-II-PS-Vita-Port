#pragma once

#include <functional>

#include "scenes/scene.hpp"

namespace defender {

class LoadingScene : public Scene {
public:
    using TransitionRequest = std::function<void(int scene_id, int direction)>;

    explicit LoadingScene(TransitionRequest transition_cb);

    bool touch(const TouchEvent& event) override;
    void draw() override;
    void update() override;
    void reset() override;

private:
    static void draw_centered(const char* path, float cx, float cy, float w, float h, float alpha = 1.0f);

    TransitionRequest transition_cb_;
    int comic_time_ms_ = 0;
    float line_x_ = -400.0f;
    float line_alpha_ = 1.0f;
    float logo_alpha_ = 0.0f;
    float dh_alpha_ = 0.0f;
    float game_alpha_ = 0.0f;
    float light_alpha_ = 0.0f;
    float all_alpha_ = 1.0f;
    bool transitioned_ = false;
};

} // namespace defender

