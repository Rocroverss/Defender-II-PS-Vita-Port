#include "scenes/research/research_components.hpp"

#include <cstddef>
#include <string>
#include <utility>

#include "game/item_param.hpp"
#include "game/save.hpp"

namespace defender {

EquipZone::EquipZone(ShowBowMenuRequest show_bow_menu_cb)
    : show_bow_menu_cb_(std::move(show_bow_menu_cb)),
      bow_(0, 150.0f, 412.0f, true),
      magic_{
          EquipButton(0, 215.0f, 412.0f, false),
          EquipButton(3, 280.0f, 412.0f, false),
          EquipButton(6, 345.0f, 412.0f, false)},
      fire_{
          EquipButton(0, 205.0f, 332.0f, false),
          EquipButton(1, 205.0f, 247.0f, false),
          EquipButton(2, 205.0f, 162.0f, false)},
      ice_{
          EquipButton(3, 270.0f, 332.0f, false),
          EquipButton(4, 270.0f, 247.0f, false),
          EquipButton(5, 270.0f, 162.0f, false)},
      light_{
          EquipButton(6, 335.0f, 332.0f, false),
          EquipButton(7, 335.0f, 247.0f, false),
          EquipButton(8, 335.0f, 162.0f, false)} {
    for (int i = 0; i < 3; ++i) {
        magic_list_[0][static_cast<size_t>(i)] = &fire_[static_cast<size_t>(i)];
        magic_list_[1][static_cast<size_t>(i)] = &ice_[static_cast<size_t>(i)];
        magic_list_[2][static_cast<size_t>(i)] = &light_[static_cast<size_t>(i)];
    }
    bow_.set_scale(0.75f);
    for (auto& magic_slot : magic_) {
        magic_slot.set_scale(0.75f);
    }
}

void EquipZone::reset_magic_lock() {
    for (int i = 0; i < 3; ++i) {
        if (ItemParam::get_level(i + 15) > 0) {
            fire_[static_cast<size_t>(i)].set_lock(false);
        }
        if (ItemParam::get_level(i + 18) > 0) {
            ice_[static_cast<size_t>(i)].set_lock(false);
        }
        if (ItemParam::get_level(i + 21) > 0) {
            light_[static_cast<size_t>(i)].set_lock(false);
        }
    }
}

void EquipZone::reset() {
    for (int i = 0; i < 3; ++i) {
        magic_[static_cast<size_t>(i)].set_lock(true);
        magic_[static_cast<size_t>(i)].release();
        fire_[static_cast<size_t>(i)].set_lock(true);
        ice_[static_cast<size_t>(i)].set_lock(true);
        light_[static_cast<size_t>(i)].set_lock(true);
    }
    press_id_ = 0;
    bow_.equip_bow(ItemParam::get_level(ItemParam::BOW_EQUIP));
    magic_[0].equip_magic(0, ItemParam::get_level(ItemParam::MAGIC1_EQUIP));
    const int magic2 = ItemParam::get_level(ItemParam::MAGIC2_EQUIP);
    if (magic2 >= 0) {
        magic_[1].equip_magic(3, magic2);
    }
    const int magic3 = ItemParam::get_level(ItemParam::MAGIC3_EQUIP);
    if (magic3 >= 0) {
        magic_[2].equip_magic(6, magic3);
    }
    reset_magic_lock();
}

void EquipZone::equip_bow(int type) {
    bow_.equip_bow(type);
}

void EquipZone::equip_magic(int type, int level) {
    if (type < 0 || type >= 3) {
        return;
    }
    magic_[static_cast<size_t>(type)].equip_magic(type * 3, level);
}

bool EquipZone::touch(const TouchEvent& event, float x1, float y1, float, float) {
    if (event.action == TouchAction::Down) {
        for (int i = 0; i < 3; ++i) {
            if (magic_[static_cast<size_t>(i)].contains(x1, y1)) {
                if (magic_[static_cast<size_t>(i)].is_pressed()) {
                    magic_[static_cast<size_t>(i)].release();
                    return true;
                }
                magic_[static_cast<size_t>(press_id_)].release();
                magic_[static_cast<size_t>(i)].press();
                press_id_ = i;
                return true;
            }
        }
        if (bow_.contains(x1, y1)) {
            if (show_bow_menu_cb_) {
                show_bow_menu_cb_();
            }
            return true;
        }
        if (magic_[static_cast<size_t>(press_id_)].is_pressed()) {
            for (int i = 0; i < 3; ++i) {
                EquipButton* btn = magic_list_[static_cast<size_t>(press_id_)][static_cast<size_t>(i)];
                if (btn->contains(x1, y1) && !btn->is_locked()) {
                    magic_[static_cast<size_t>(press_id_)].equip_magic(press_id_ * 3, i);
                    magic_[static_cast<size_t>(press_id_)].release();
                    ItemParam::init_level(press_id_ + 25, i);
                    Save::save_data(std::string(Save::EQUIPED_MAGIC) + std::to_string(press_id_), i);
                    return true;
                }
            }
            magic_[static_cast<size_t>(press_id_)].release();
            return true;
        }
    }
    return false;
}

void EquipZone::draw() {
    bow_.draw();
    for (auto& slot : magic_) {
        slot.draw();
    }
    if (magic_[static_cast<size_t>(press_id_)].is_pressed()) {
        for (int i = 0; i < 3; ++i) {
            magic_list_[static_cast<size_t>(press_id_)][static_cast<size_t>(i)]->draw();
        }
    }
}

} // namespace defender
