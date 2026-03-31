#include "scenes/research/research_components.hpp"

#include "scenes/research/research_render.hpp"

namespace defender {

BowButton::BowButton(const std::string& frame_name, float x, float y)
    : frame_name_(frame_name), x_(sx(x)), y_(sy(y)) {
    w_ = frame_width(frame_name_, 1.0f, sx(64.0f));
    h_ = frame_height(frame_name_, 1.0f, sy(64.0f));
}

void BowButton::draw(float offset_x, float offset_y, float show_x) const {
    const float draw_x = offset_x + x_ - show_x;
    const float draw_y = offset_y + y_;
    if (is_lock_) {
        draw_frame("icon_lock.png", draw_x, draw_y, 1.0f, 1.0f);
    } else {
        if (!is_get_) {
            draw_frame_tinted(frame_name_, draw_x, draw_y, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f);
        } else {
            draw_frame(frame_name_, draw_x, draw_y, 1.0f, 1.0f);
        }
    }
    if (is_select_) {
        draw_frame("icon_select.png", draw_x, draw_y, 1.0f, 1.0f);
    }
}

bool BowButton::contains(float local_x, float local_y) const {
    return local_x >= x_ && local_x <= x_ + w_ && local_y >= y_ && local_y <= y_ + h_;
}

void BowButton::press() {
    is_select_ = true;
}

void BowButton::release() {
    is_select_ = false;
}

bool BowButton::is_pressed() const {
    return is_select_;
}

void BowButton::set_get(bool is_get) {
    is_get_ = is_get;
}

bool BowButton::is_get() const {
    return is_get_;
}

void BowButton::set_lock(bool is_lock) {
    is_lock_ = is_lock;
}

bool BowButton::is_lock() const {
    return is_lock_;
}

} // namespace defender

