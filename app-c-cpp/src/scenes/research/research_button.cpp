#include "scenes/research/research_components.hpp"

#include <algorithm>
#include <string>

#include "engine/abstract_game.hpp"
#include "engine/texture_cache.hpp"
#include "scenes/research/research_render.hpp"

namespace defender {
namespace {

constexpr const char* kLevelNumberStrip = "assets/imgs_480_800/game/z_number_list_level.png";

float number_width(int value, float scale) {
    int safe = value;
    if (safe < 0) {
        safe = 0;
    }
    const auto& tex = TextureCache::instance().get(kLevelNumberStrip);
    if (!tex.valid || tex.width <= 0) {
        return 0.0f;
    }
    const float digit_w = (static_cast<float>(tex.width) / 10.0f) * scale;
    return static_cast<float>(std::to_string(safe).size()) * digit_w;
}

} // namespace

ResearchButton::ResearchButton(const std::string& frame_name, float x, float y, int max_level)
    : frame_name_(frame_name), x_(sx(x)), y_(sy(y)), max_level_(max_level) {
    w_ = frame_width(frame_name_, 1.0f, sx(100.0f));
    h_ = frame_height(frame_name_, 1.0f, sy(100.0f));
}

bool ResearchButton::is_max_level() const {
    return level_ >= max_level_;
}

void ResearchButton::set_add_level(int level) {
    add_level_ = level;
}

int ResearchButton::get_add_level() const {
    return add_level_;
}

void ResearchButton::set_level(int level) {
    level_ = level;
}

int ResearchButton::get_level() const {
    return level_;
}

void ResearchButton::set_lock(bool is_lock) {
    is_locked_ = is_lock;
}

bool ResearchButton::is_locked() const {
    return is_locked_;
}

bool ResearchButton::is_show_upgrade() const {
    return show_time_ms_ > 0;
}

void ResearchButton::upgrade() {
    set_level(level_ + 1);
    show_time_ms_ = 500;
}

void ResearchButton::set_frame(const std::string& frame_name) {
    frame_name_ = frame_name;
    w_ = frame_width(frame_name_, 1.0f, w_);
    h_ = frame_height(frame_name_, 1.0f, h_);
}

void ResearchButton::press() {
    is_select_ = true;
}

void ResearchButton::release() {
    is_select_ = false;
}

bool ResearchButton::contains(float local_x, float local_y) const {
    return local_x >= x_ && local_x <= x_ + w_ && local_y >= y_ && local_y <= y_ + h_;
}

void ResearchButton::draw(float offset_x, float offset_y, float show_x) {
    const float draw_x = offset_x + x_ - show_x;
    const float draw_y = offset_y + y_;
    const float temp = static_cast<float>(500 - show_time_ms_) / 500.0f;
    const float alpha = std::clamp(temp, 0.0f, 1.0f);

    draw_frame(frame_name_, draw_x, draw_y, 1.0f, alpha);

    if (level_ > 0) {
        const float number_y = draw_y + sy(7.0f);
        const float right_x = draw_x + sx(58.0f);
        const float width = number_width(level_, 1.0f);
        draw_number_strip(kLevelNumberStrip, level_, right_x - width, number_y, 1.0f, 1.0f, 1.0f, 1.0f, alpha);
    }
    if (add_level_ > 0) {
        const float number_y = draw_y + sy(42.0f);
        const float right_x = draw_x + sx(58.0f);
        const float width = number_width(add_level_, 1.0f);
        draw_number_strip(kLevelNumberStrip, add_level_, right_x - width, number_y, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    }
    if (is_locked_) {
        draw_frame("logo_locked.png", draw_x, draw_y, 1.0f, alpha);
    }
    if (is_select_) {
        draw_frame("logo_select.png", draw_x, draw_y, 1.0f, alpha);
    }

    if (show_time_ms_ > 0) {
        const int dt = static_cast<int>(AbstractGame::last_span_ms());
        show_time_ms_ -= dt;
        if (show_time_ms_ < 0) {
            show_time_ms_ = 0;
        }
    }
}

} // namespace defender

