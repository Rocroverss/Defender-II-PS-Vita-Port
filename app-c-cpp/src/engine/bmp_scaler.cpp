#include "engine/bmp_scaler.hpp"

#include <algorithm>

#include "engine/screen.hpp"

namespace defender {

BmpScaler& BmpScaler::instance() {
    static BmpScaler scaler;
    return scaler;
}

void BmpScaler::set_bitmap_original(float width, float height) {
    original_width_ = width;
    original_height_ = height;
    inited_ = true;
    update_scale();
}

void BmpScaler::update_scale() {
    if (!inited_) {
        return;
    }

    const auto& screen = Screen::current();
    scale_x_ = static_cast<float>(screen.width()) / original_width_;
    scale_y_ = static_cast<float>(screen.height()) / original_height_;
    scale_min_ = std::min(scale_x_, scale_y_);
}

float BmpScaler::scale_x(float width, ScaleType scale_type) const {
    return (scale_type == ScaleType::FitScreen) ? (scale_x_ * width) : (scale_min_ * width);
}

float BmpScaler::scale_y(float height, ScaleType scale_type) const {
    return (scale_type == ScaleType::FitScreen) ? (scale_y_ * height) : (scale_min_ * height);
}

}

