#include "scenes/solid_scene.hpp"

#include "engine/abstract_game.hpp"
#include "engine/render_utils.hpp"

namespace defender {

SolidScene::SolidScene(float r, float g, float b) : r_(r), g_(g), b_(b) {}

void SolidScene::draw() {
    float p = pulse_;
    if (p > 1.0f) {
        p = 2.0f - p;
    }
    const float accent = touched_ ? (0.25f + 0.35f * p) : 0.15f;

    draw_quad(0.0f, 0.0f, screen_width, screen_height, r_, g_, b_, 1.0f);
    draw_quad(get_x(20.0f), get_y(20.0f), get_x(760.0f), get_y(440.0f), 1.0f, 1.0f, 1.0f, accent);
}

bool SolidScene::touch(const TouchEvent& event) {
    if (event.action == TouchAction::Down) {
        touched_ = true;
        pulse_ = 0.0f;
    } else if (event.action == TouchAction::Up) {
        touched_ = false;
    }
    return true;
}

void SolidScene::update() {
    if (!touched_) {
        return;
    }
    pulse_ += static_cast<float>(AbstractGame::last_span_ms()) / 500.0f;
    if (pulse_ > 2.0f) {
        pulse_ -= 2.0f;
    }
}

void SolidScene::reset() {
    touched_ = false;
    pulse_ = 0.0f;
    scene_x = 0.0f;
    scene_y = 0.0f;
    scene_scale = 1.0f;
    scene_alpha = 1.0f;
}

}

