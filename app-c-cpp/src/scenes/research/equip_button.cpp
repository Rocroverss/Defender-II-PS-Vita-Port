#include "scenes/research/research_components.hpp"

#include <algorithm>
#include <array>
#include <cstddef>

#include "engine/abstract_game.hpp"
#include "scenes/research/research_render.hpp"

namespace defender {
namespace {

const std::array<const char*, 29> kBowFrames = {
    "icon_normal.png",
    "icon_pow_01.png",
    "icon_pow_02.png",
    "icon_pow_03.png",
    "icon_pow_04.png",
    "icon_pow_05.png",
    "icon_pow_06.png",
    "icon_pow_07.png",
    "icon_pow_08.png",
    "icon_pow_09.png",
    "icon_agi_01.png",
    "icon_agi_02.png",
    "icon_agi_03.png",
    "icon_agi_04.png",
    "icon_agi_05.png",
    "icon_agi_06.png",
    "icon_agi_07.png",
    "icon_agi_08.png",
    "icon_agi_09.png",
    "icon_multi_01.png",
    "icon_multi_02.png",
    "icon_multi_03.png",
    "icon_multi_04.png",
    "icon_multi_05.png",
    "icon_multi_06.png",
    "icon_multi_07.png",
    "icon_multi_08.png",
    "icon_multi_09.png",
    "icon_super.png"
};

const std::array<const char*, 9> kMagicFrames = {
    "icon_fire_01.png",
    "icon_fire_02.png",
    "icon_fire_03.png",
    "icon_ice_01.png",
    "icon_ice_02.png",
    "icon_ice_03.png",
    "icon_light_01.png",
    "icon_light_02.png",
    "icon_light_03.png"
};

std::string bow_frame(int id) {
    if (id < 0 || id >= static_cast<int>(kBowFrames.size())) {
        return "icon_normal.png";
    }
    return kBowFrames[static_cast<size_t>(id)];
}

std::string magic_frame(int id) {
    if (id < 0 || id >= static_cast<int>(kMagicFrames.size())) {
        return "icon_fire_01.png";
    }
    return kMagicFrames[static_cast<size_t>(id)];
}

} // namespace

EquipButton::EquipButton(int img_id, float x, float y, bool is_bow)
    : x_(sx(x)), y_(sy(y)), is_bow_(is_bow), img_id_(img_id) {
    frame_name_ = is_bow_ ? bow_frame(img_id_) : magic_frame(img_id_);
    update_rect();
}

void EquipButton::set_lock(bool is_locked) {
    is_locked_ = is_locked;
}

bool EquipButton::is_locked() const {
    return is_locked_;
}

void EquipButton::set_scale(float scale) {
    scale_ = scale;
    update_rect();
}

bool EquipButton::is_show_equip() const {
    return show_time_ms_ > 0;
}

void EquipButton::equip_magic(int type, int level) {
    show_time_ms_ = 500;
    frame_name_ = magic_frame(type + level);
    is_locked_ = false;
    update_rect();
}

void EquipButton::equip_bow(int bow_id) {
    show_time_ms_ = 500;
    frame_name_ = bow_frame(bow_id);
    is_locked_ = false;
    update_rect();
}

bool EquipButton::is_pressed() const {
    return is_pressed_;
}

void EquipButton::press() {
    is_pressed_ = true;
}

void EquipButton::release() {
    is_pressed_ = false;
}

bool EquipButton::contains(float x, float y) const {
    return x >= x_ && x <= x_ + w_ && y >= y_ && y <= y_ + h_;
}

void EquipButton::update_rect() {
    w_ = frame_width(frame_name_, scale_, sx(64.0f) * scale_);
    h_ = frame_height(frame_name_, scale_, sy(64.0f) * scale_);
}

void EquipButton::draw() {
    const float temp = static_cast<float>(500 - show_time_ms_) / 500.0f;
    const float alpha = std::clamp(temp, 0.0f, 1.0f);
    if (is_locked_) {
        draw_frame("icon_lock.png", x_, y_, scale_, alpha);
    } else {
        draw_frame(frame_name_, x_, y_, scale_, alpha);
    }
    if (show_time_ms_ > 0) {
        show_time_ms_ -= static_cast<int>(AbstractGame::last_span_ms());
        if (show_time_ms_ < 0) {
            show_time_ms_ = 0;
        }
    }
}

} // namespace defender
