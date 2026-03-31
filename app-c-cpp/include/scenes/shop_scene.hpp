#pragma once

#include <functional>

#include "scenes/scene.hpp"

namespace defender {

class ShopScene : public Scene {
public:
    using TransitionRequest = std::function<void(int scene_id, int direction)>;

    struct ShopButton {
        const char* up = nullptr;
        const char* down = nullptr;
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
    };

    explicit ShopScene(TransitionRequest transition_cb);

    bool touch(const TouchEvent& event) override;
    void draw() override;
    void update() override;
    void reset() override;

private:
    static bool hit_test(const ShopButton& button, float px, float py);
    static void draw_button(const ShopButton& button, bool pressed, float alpha = 1.0f);

    void purchase(int id);

    TransitionRequest transition_cb_;
    int pressed_id_ = -1;
    float pulse_ = 0.0f;
};

} // namespace defender
