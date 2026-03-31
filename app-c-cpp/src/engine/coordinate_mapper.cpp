#include "engine/coordinate_mapper.hpp"

#include <algorithm>

#include "engine/screen.hpp"

namespace defender {

CoordinateMapper& CoordinateMapper::instance() {
    static CoordinateMapper mapper;
    return mapper;
}

void CoordinateMapper::set_designed(float designed_width, float designed_height) {
    designed_width_ = designed_width;
    designed_height_ = designed_height;
    inited_ = true;
    update_scale();
}

void CoordinateMapper::update_scale() {
    if (!inited_) {
        return;
    }

    const auto& screen = Screen::current();
    scale_x_ = static_cast<float>(screen.width()) / designed_width_;
    scale_y_ = static_cast<float>(screen.height()) / designed_height_;
    scale_min_ = std::min(scale_x_, scale_y_);
}

float CoordinateMapper::gen_game_length(float length_in_designed_screen) const {
    return scale_min_ * length_in_designed_screen;
}

float CoordinateMapper::gen_game_length_x(float length_in_designed_screen) const {
    return scale_x_ * length_in_designed_screen;
}

float CoordinateMapper::gen_game_length_y(float length_in_designed_screen) const {
    return scale_y_ * length_in_designed_screen;
}

}

