#include "scenes/research/research_components.hpp"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <string>
#include <utility>

#include "engine/abstract_game.hpp"
#include "engine/render_utils.hpp"
#include "game/achv_mng.hpp"
#include "game/audio_manager.hpp"
#include "game/bow_data.hpp"
#include "game/help_overlay.hpp"
#include "game/item_param.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "game/sounds.hpp"
#include "scenes/research/research_render.hpp"
#include "scenes/scene.hpp"

namespace defender {
namespace {

constexpr const char* kItemCover = "assets/imgs_480_800/research/equip_cover_bg_fixed.png";
constexpr const char* kDefenderLine = "assets/imgs_480_800/research/research_defender_line.png";
constexpr const char* kMagicLine = "assets/imgs_480_800/research/research_magic_line.png";
constexpr const char* kWallLine = "assets/imgs_480_800/research/research_wall_line.png";

constexpr float kDefenderLineW = 599.0f;
constexpr float kDefenderLineH = 192.0f;
constexpr float kMagicLineW = 600.0f;
constexpr float kMagicLineH = 194.0f;
constexpr float kWallLineW = 331.0f;
constexpr float kWallLineH = 228.0f;
constexpr float kItemCoverCropX = 0.0f;
constexpr float kItemCoverCropY = 79.0f;
constexpr float kItemCoverCropW = 800.0f;
constexpr float kItemCoverCropH = 299.0f;

void draw_research_layout_texture(
    const char* path,
    float x,
    float y,
    float w,
    float h,
    ResearchLayoutElement element,
    bool use_logical_draw = false
) {
    const auto& layout = research_layout_tweak(element);
    const float draw_x = x + sx(layout.offset_x);
    const float draw_y = y + sy(layout.offset_y);
    const float draw_w = sx(w * layout.scale);
    const float draw_h = sy(h * layout.scale);

    if (use_logical_draw) {
        draw_texture_logical(path, draw_x, draw_y, draw_w, draw_h);
        return;
    }
    draw_texture(path, draw_x, draw_y, draw_w, draw_h, 1.0f);
}

void draw_research_layout_texture_region(
    const char* path,
    float x,
    float y,
    float w,
    float h,
    float src_x,
    float src_y,
    float src_w,
    float src_h,
    ResearchLayoutElement element
) {
    const auto& layout = research_layout_tweak(element);
    const float draw_x = x + sx(layout.offset_x);
    const float draw_y = y + sy(layout.offset_y);
    const float draw_w = sx(w * layout.scale);
    const float draw_h = sy(h * layout.scale);
    draw_texture_logical_region(path, draw_x, draw_y, draw_w, draw_h, src_x, src_y, src_w, src_h);
}

} // namespace

ItemZone::ItemZone(EquipZone* equip_zone, OpenShopRequest open_shop_cb, BowFlashRequest bow_flash_cb)
    : open_shop_cb_(std::move(open_shop_cb)),
      bow_flash_cb_(std::move(bow_flash_cb)),
      equip_zone_(equip_zone),
      des_(this),
      up_area_(this) {
    origin_x_ = sx(80.0f);
    origin_y_ = sy(100.0f);

    item_list_[7] = std::make_unique<ResearchButton>("logo_strength.png", 30.0f, 175.0f, 205);
    item_list_[8] = std::make_unique<ResearchButton>("logo_agility.png", 30.0f, 65.0f, 30);
    item_list_[9] = std::make_unique<ResearchButton>("logo_power_shot.png", 300.0f, 210.0f, 9);
    item_list_[13] = std::make_unique<ResearchButton>("logo_poisoned_arrow.png", 300.0f, 120.0f, 9);
    item_list_[10] = std::make_unique<ResearchButton>("logo_fatal_blow.png", 300.0f, 30.0f, 15);
    item_list_[11] = std::make_unique<ResearchButton>("logo_multiple_arrows.png", 570.0f, 120.0f, 30);
    item_list_[12] = std::make_unique<ResearchButton>("logo_senior_hunter.png", 840.0f, 120.0f, 9);
    item_list_[14] = std::make_unique<ResearchButton>("logo_mana_research.png", 30.0f, 120.0f, 30);
    item_list_[15] = std::make_unique<ResearchButton>("logo_fire_1.png", 300.0f, 30.0f, 9);
    item_list_[18] = std::make_unique<ResearchButton>("logo_ice_1.png", 300.0f, 120.0f, 9);
    item_list_[21] = std::make_unique<ResearchButton>("logo_light_1.png", 300.0f, 210.0f, 9);
    item_list_[16] = std::make_unique<ResearchButton>("logo_fire_2.png", 570.0f, 30.0f, 9);
    item_list_[19] = std::make_unique<ResearchButton>("logo_ice_2.png", 570.0f, 120.0f, 9);
    item_list_[22] = std::make_unique<ResearchButton>("logo_light_2.png", 570.0f, 210.0f, 9);
    item_list_[17] = std::make_unique<ResearchButton>("logo_fire_3.png", 840.0f, 30.0f, 30);
    item_list_[20] = std::make_unique<ResearchButton>("logo_ice_3.png", 840.0f, 120.0f, 30);
    item_list_[23] = std::make_unique<ResearchButton>("logo_light_3.png", 840.0f, 210.0f, 30);
    item_list_[0] = std::make_unique<ResearchButton>("logo_city_wall.png", 30.0f, 120.0f, 23);
    item_list_[4] = std::make_unique<ResearchButton>("logo_magic_tower.png", 300.0f, 185.0f, 2);
    item_list_[1] = std::make_unique<ResearchButton>("logo_lava_moat.png", 300.0f, 55.0f, 3);
    item_list_[5] = std::make_unique<ResearchButton>("logo_magic_power.png", 570.0f, 228.0f, 99);
    item_list_[6] = std::make_unique<ResearchButton>("logo_splash.png", 570.0f, 156.0f, 20);
    item_list_[2] = std::make_unique<ResearchButton>("logo_burn.png", 570.0f, 84.0f, 99);
    item_list_[3] = std::make_unique<ResearchButton>("logo_entangling_lava.png", 570.0f, 12.0f, 20);

    bow_list_[0] = std::make_unique<BowButton>("icon_normal.png", 30.0f, 110.0f);
    bow_list_[10] = std::make_unique<BowButton>("icon_agi_01.png", 140.0f, 110.0f);
    bow_list_[11] = std::make_unique<BowButton>("icon_agi_02.png", 250.0f, 110.0f);
    bow_list_[12] = std::make_unique<BowButton>("icon_agi_03.png", 360.0f, 110.0f);
    bow_list_[13] = std::make_unique<BowButton>("icon_agi_04.png", 470.0f, 110.0f);
    bow_list_[14] = std::make_unique<BowButton>("icon_agi_05.png", 580.0f, 110.0f);
    bow_list_[15] = std::make_unique<BowButton>("icon_agi_06.png", 690.0f, 110.0f);
    bow_list_[16] = std::make_unique<BowButton>("icon_agi_07.png", 800.0f, 110.0f);
    bow_list_[17] = std::make_unique<BowButton>("icon_agi_08.png", 910.0f, 110.0f);
    bow_list_[18] = std::make_unique<BowButton>("icon_agi_09.png", 1020.0f, 110.0f);
    bow_list_[1] = std::make_unique<BowButton>("icon_pow_01.png", 140.0f, 205.0f);
    bow_list_[2] = std::make_unique<BowButton>("icon_pow_02.png", 250.0f, 205.0f);
    bow_list_[3] = std::make_unique<BowButton>("icon_pow_03.png", 360.0f, 205.0f);
    bow_list_[4] = std::make_unique<BowButton>("icon_pow_04.png", 470.0f, 205.0f);
    bow_list_[5] = std::make_unique<BowButton>("icon_pow_05.png", 580.0f, 205.0f);
    bow_list_[6] = std::make_unique<BowButton>("icon_pow_06.png", 690.0f, 205.0f);
    bow_list_[7] = std::make_unique<BowButton>("icon_pow_07.png", 800.0f, 205.0f);
    bow_list_[8] = std::make_unique<BowButton>("icon_pow_08.png", 910.0f, 205.0f);
    bow_list_[9] = std::make_unique<BowButton>("icon_pow_09.png", 1020.0f, 205.0f);
    bow_list_[19] = std::make_unique<BowButton>("icon_multi_01.png", 140.0f, 15.0f);
    bow_list_[20] = std::make_unique<BowButton>("icon_multi_02.png", 250.0f, 15.0f);
    bow_list_[21] = std::make_unique<BowButton>("icon_multi_03.png", 360.0f, 15.0f);
    bow_list_[22] = std::make_unique<BowButton>("icon_multi_04.png", 470.0f, 15.0f);
    bow_list_[23] = std::make_unique<BowButton>("icon_multi_05.png", 580.0f, 15.0f);
    bow_list_[24] = std::make_unique<BowButton>("icon_multi_06.png", 690.0f, 15.0f);
    bow_list_[25] = std::make_unique<BowButton>("icon_multi_07.png", 800.0f, 15.0f);
    bow_list_[26] = std::make_unique<BowButton>("icon_multi_08.png", 910.0f, 15.0f);
    bow_list_[27] = std::make_unique<BowButton>("icon_multi_09.png", 1020.0f, 15.0f);
    bow_list_[28] = std::make_unique<BowButton>("icon_super.png", 1140.0f, 110.0f);

    set_target_id(7);
}

ResearchButton* ItemZone::get_item(int type) {
    if (type < 0 || type >= ITEM_SIZE) {
        return nullptr;
    }
    return item_list_[static_cast<size_t>(type)].get();
}

BowButton* ItemZone::get_bow(int type) {
    if (type < 0 || type >= BOW_SIZE) {
        return nullptr;
    }
    return bow_list_[static_cast<size_t>(type)].get();
}

int ItemZone::get_add_level(int type) const {
    if (type < 0 || type >= ITEM_SIZE) {
        return 0;
    }
    const auto& item = item_list_[static_cast<size_t>(type)];
    return item ? item->get_add_level() : 0;
}

void ItemZone::equip_bow(int type) {
    Save::save_data(Save::EQUIPED_BOW, type);
    ItemParam::init_level(ItemParam::BOW_EQUIP, type);
    if (equip_zone_ != nullptr) {
        equip_zone_->equip_bow(type);
    }
    item_list_[7]->set_add_level(BowData::get_ability(type, 0));
    item_list_[8]->set_add_level(BowData::get_ability(type, 1));
    item_list_[9]->set_add_level(BowData::get_ability(type, 2));
    item_list_[10]->set_add_level(BowData::get_ability(type, 3));
    item_list_[11]->set_add_level(BowData::get_ability(type, 4));
    AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
}

void ItemZone::get_pack_final_fantasy_bow() {
    bow_list_[28]->set_get(true);
    Save::save_data(std::string(Save::BOW_GET) + "28", 1);
    up_area_.set_type(28, true);
}

void ItemZone::buy_bow(int type, int cost, bool is_gold) {
    if (is_gold) {
        if (Param::gold >= cost) {
            bow_list_[static_cast<size_t>(type)]->set_get(true);
            AudioManager::instance().play_sound(Sounds::BUTTON_UPGREADE);
            Save::save_data(std::string(Save::BOW_GET) + std::to_string(type), 1);
            cost_act(cost, true);
            equip_bow(type);
        } else if (open_shop_cb_) {
            open_shop_cb_();
        }
    } else if (Param::stone >= cost) {
        bow_list_[static_cast<size_t>(type)]->set_get(true);
        AudioManager::instance().play_sound(Sounds::BUTTON_UPGREADE);
        Save::save_data(std::string(Save::BOW_GET) + std::to_string(type), 1);
        cost_act(cost, false);
        equip_bow(type);
    } else if (open_shop_cb_) {
        open_shop_cb_();
    }
    up_area_.set_type(type, true);
}

void ItemZone::cost_act(int cost, bool is_coin) {
    if (is_coin) {
        Param::gold -= cost;
        Param::cost_coin += cost;
        Save::save_data(Save::GOLD, Param::gold);
        Save::save_data(Save::COST_COIN, Param::cost_coin);
        AchvMng::check_achv_in_game(0);
        return;
    }
    Param::stone -= cost;
    Save::save_data(Save::STONE, Param::stone);
}

void ItemZone::upgrade(int type, int cost, bool is_gold) {
    if (item_list_[static_cast<size_t>(type)]->is_show_upgrade()) {
        return;
    }
    if (is_gold) {
        if (Param::gold >= cost) {
            item_list_[static_cast<size_t>(type)]->upgrade();
            ItemParam::init_level(type, ItemParam::get_level(type) + 1);
            AudioManager::instance().play_sound(Sounds::BUTTON_UPGREADE);
            Save::save_data(std::string(Save::SKILL_LEVEL) + std::to_string(type), ItemParam::get_level(type));
            cost_act(cost, true);
        } else {
            if (open_shop_cb_) {
                open_shop_cb_();
            }
            return;
        }
    } else if (Param::stone >= cost) {
        item_list_[static_cast<size_t>(type)]->upgrade();
        ItemParam::init_level(type, ItemParam::get_level(type) + 1);
        AudioManager::instance().play_sound(Sounds::BUTTON_UPGREADE);
        Save::save_data(std::string(Save::SKILL_LEVEL) + std::to_string(type), ItemParam::get_level(type));
        cost_act(cost, false);
    } else {
        if (open_shop_cb_) {
            open_shop_cb_();
        }
        return;
    }

    if ((type == 19 || type == 16 || type == 22) && item_list_[static_cast<size_t>(type)]->get_level() == 1) {
        HelpOverlay::set_help(14);
    }
    if (type == 18 && item_list_[18]->get_level() == 1) {
        if (equip_zone_ != nullptr) {
            equip_zone_->equip_magic(1, 0);
        }
        Save::save_data(std::string(Save::EQUIPED_MAGIC) + "1", 0);
        ItemParam::init_level(ItemParam::MAGIC2_EQUIP, 0);
    }
    if (type == 21 && item_list_[21]->get_level() == 1) {
        if (equip_zone_ != nullptr) {
            equip_zone_->equip_magic(2, 0);
        }
        Save::save_data(std::string(Save::EQUIPED_MAGIC) + "2", 0);
        ItemParam::init_level(ItemParam::MAGIC3_EQUIP, 0);
    }
    if (equip_zone_ != nullptr) {
        equip_zone_->reset_magic_lock();
    }
    unlock_check();
    set_target_id(type);
}

void ItemZone::set_item_type(int type) {
    target_type_ = type;
}

void ItemZone::set_item_type_in_draw(int type) {
    item_type_ = type;
    show_x_ = 0.0f;
    if (item_list_[static_cast<size_t>(press_id_)]) {
        item_list_[static_cast<size_t>(press_id_)]->release();
    }
    if (bow_list_[static_cast<size_t>(bow_press_id_)]) {
        bow_list_[static_cast<size_t>(bow_press_id_)]->release();
    }
    move_speed_ = 0.0f;
    switch (type) {
    case 0:
        show_start_id_ = 7;
        show_end_id_ = 13;
        show_x_min_ = 0.0f;
        show_x_max_ = sx(450.0f);
        set_target_id(7);
        item_list_[static_cast<size_t>(press_id_)]->press();
        break;
    case 1:
        show_start_id_ = 14;
        show_end_id_ = 23;
        show_x_min_ = 0.0f;
        show_x_max_ = sx(450.0f);
        set_target_id(14);
        item_list_[static_cast<size_t>(press_id_)]->press();
        break;
    case 2:
        show_start_id_ = 0;
        show_end_id_ = 6;
        show_x_min_ = 0.0f;
        show_x_max_ = sx(180.0f);
        set_target_id(0);
        item_list_[static_cast<size_t>(press_id_)]->press();
        break;
    case 3:
        show_start_id_ = 0;
        show_end_id_ = 28;
        show_x_min_ = 0.0f;
        show_x_max_ = sx(550.0f);
        set_target_id(ItemParam::get_level(ItemParam::BOW_EQUIP));
        bow_list_[static_cast<size_t>(bow_press_id_)]->press();
        break;
    default:
        break;
    }
}

bool ItemZone::touch(const TouchEvent& event, float x1, float y1, float, float) {
    if (!up_area_.touch(event, x1, y1, 0.0f, 0.0f)) {
        return false;
    }

    RectF zone = make_rect(sx(80.0f), sy(100.0f), sx(720.0f), sy(300.0f));

    switch (event.action) {
    case TouchAction::Down:
        if (zone.contains(x1, y1)) {
            move_flag_ = true;
            cur_x_ = x1;
            pas_x_ = x1;
            pas_time_ms_ = AbstractGame::game_time_ms();
            if (item_type_ == 3) {
                for (int i = show_start_id_; i <= show_end_id_; ++i) {
                    if (bow_list_[static_cast<size_t>(i)]->contains((show_x_ + x1) - origin_x_, y1 - origin_y_)) {
                        bow_list_[static_cast<size_t>(bow_press_id_)]->release();
                        set_target_id(i);
                        bow_list_[static_cast<size_t>(i)]->press();
                        return false;
                    }
                }
            } else {
                for (int i = show_start_id_; i <= show_end_id_; ++i) {
                    if (item_list_[static_cast<size_t>(i)]->contains((show_x_ + x1) - origin_x_, y1 - origin_y_)) {
                        item_list_[static_cast<size_t>(press_id_)]->release();
                        set_target_id(i);
                        item_list_[static_cast<size_t>(i)]->press();
                        return false;
                    }
                }
            }
        }
        break;
    case TouchAction::Up:
        if (move_flag_) {
            cur_time_ms_ = AbstractGame::game_time_ms();
            if (cur_time_ms_ > pas_time_ms_ && (cur_time_ms_ - pas_time_ms_) > 300) {
                move_speed_ = 0.0f;
            }
            move_flag_ = false;
        }
        break;
    case TouchAction::Move:
        if (move_flag_) {
            cur_x_ = x1;
            cur_time_ms_ = AbstractGame::game_time_ms();
            add_show_x(pas_x_ - cur_x_);
            if (cur_time_ms_ > pas_time_ms_) {
                move_speed_ = ((pas_x_ - cur_x_) * 1000.0f) / static_cast<float>(cur_time_ms_ - pas_time_ms_);
            }
            pas_time_ms_ = cur_time_ms_;
            pas_x_ = cur_x_;
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}

void ItemZone::set_target_id(int id) {
    if (item_type_ == 3) {
        bow_press_id_ = id;
        des_.set_des_type(id, true);
        up_area_.set_type(id, true);
    } else {
        press_id_ = id;
        des_.set_des_type(id, false);
        up_area_.set_type(id, false);
    }
}

void ItemZone::reset() {
    for (int i = 0; i < ITEM_SIZE; ++i) {
        item_list_[static_cast<size_t>(i)]->set_level(ItemParam::get_level(i));
        if (item_list_[static_cast<size_t>(i)]->get_level() <= 0) {
            item_list_[static_cast<size_t>(i)]->set_lock(true);
        }
    }
    bow_reset();
    unlock_check();
    equip_bow(ItemParam::get_level(ItemParam::BOW_EQUIP));
    set_item_type(0);
    set_item_type_in_draw(0);
}

void ItemZone::add_show_x(float x) {
    show_x_ += x;
    if (show_x_ < show_x_min_) {
        show_x_ = show_x_min_;
    }
    if (show_x_ > show_x_max_) {
        show_x_ = show_x_max_;
    }
}

void ItemZone::bow_reset() {
    for (int i = 0; i < BOW_SIZE; ++i) {
        if (Save::load_data(std::string(Save::BOW_GET) + std::to_string(i)) == 1) {
            bow_list_[static_cast<size_t>(i)]->set_get(true);
        }
        if (i > 0 && i < 28) {
            bow_list_[static_cast<size_t>(i)]->set_lock(true);
        }
    }
    if (Param::level >= 2) {
        bow_list_[1]->set_lock(false);
        bow_list_[10]->set_lock(false);
        bow_list_[19]->set_lock(false);
    }
    for (int i = 1; i < 9; ++i) {
        if (Param::level >= i * 5) {
            bow_list_[static_cast<size_t>(i + 1)]->set_lock(false);
            bow_list_[static_cast<size_t>(i + 10)]->set_lock(false);
            bow_list_[static_cast<size_t>(i + 19)]->set_lock(false);
        }
    }
    if (Param::level <= 40) {
        if ((Param::level == 2 || Param::level % 5 == 0) && Save::load_data(std::string(Save::BOW_LOCK) + std::to_string(Param::level)) == 0) {
            if (bow_flash_cb_) {
                bow_flash_cb_();
            }
            Save::save_data(std::string(Save::BOW_LOCK) + std::to_string(Param::level), 1);
        }
    }
}

void ItemZone::unlock_check() {
    if (item_list_[7]->get_level() >= 5) item_list_[9]->set_lock(false);
    if (item_list_[8]->get_level() >= 5) item_list_[10]->set_lock(false);
    if (item_list_[7]->get_level() >= 10 && item_list_[8]->get_level() >= 10) item_list_[13]->set_lock(false);
    if (item_list_[9]->get_level() >= 3 && item_list_[10]->get_level() >= 3) item_list_[11]->set_lock(false);
    if (item_list_[11]->get_level() >= 2) item_list_[12]->set_lock(false);
    if (item_list_[14]->get_level() >= 6) {
        if (item_list_[16]->get_level() >= 3) item_list_[17]->set_lock(false);
        if (item_list_[19]->get_level() >= 3) item_list_[20]->set_lock(false);
        if (item_list_[22]->get_level() >= 3) item_list_[23]->set_lock(false);
    }
    if (item_list_[14]->get_level() >= 4) {
        if (item_list_[15]->get_level() >= 3) item_list_[16]->set_lock(false);
        if (item_list_[18]->get_level() >= 3) item_list_[19]->set_lock(false);
        if (item_list_[21]->get_level() >= 3) item_list_[22]->set_lock(false);
    }
    if (item_list_[14]->get_level() >= 2) {
        item_list_[18]->set_lock(false);
        item_list_[21]->set_lock(false);
    }
    if (item_list_[0]->get_level() >= 3) {
        item_list_[4]->set_lock(false);
        item_list_[1]->set_lock(false);
    }
    if (item_list_[4]->get_level() >= 1) {
        item_list_[5]->set_lock(false);
        item_list_[6]->set_lock(false);
    }
    if (item_list_[1]->get_level() >= 1) {
        item_list_[2]->set_lock(false);
        item_list_[3]->set_lock(false);
    }
}

void ItemZone::draw() {
    if (target_type_ != item_type_) {
        set_item_type_in_draw(target_type_);
    }

    const float line_x[4] = {232.0f, 232.0f, 232.0f, 0.0f};
    const float line_y[4] = {56.0f, 54.0f, 36.0f, 0.0f};

    switch (item_type_) {
    case 0:
    case 1:
    case 2:
        for (int i = show_start_id_; i <= show_end_id_; ++i) {
            item_list_[static_cast<size_t>(i)]->draw(origin_x_, origin_y_, show_x_);
        }
        if (item_type_ == 0) {
            draw_research_layout_texture(
                kDefenderLine,
                origin_x_ + sx(line_x[item_type_]) - show_x_,
                origin_y_ + sy(line_y[item_type_]),
                kDefenderLineW,
                kDefenderLineH,
                ResearchLayoutElement::DefenderLine,
                true
            );
        } else if (item_type_ == 1) {
            draw_research_layout_texture(
                kMagicLine,
                origin_x_ + sx(line_x[item_type_]) - show_x_,
                origin_y_ + sy(line_y[item_type_]),
                kMagicLineW,
                kMagicLineH,
                ResearchLayoutElement::MagicLine,
                true
            );
        } else {
            draw_research_layout_texture(
                kWallLine,
                origin_x_ + sx(line_x[item_type_]) - show_x_,
                origin_y_ + sy(line_y[item_type_]),
                kWallLineW,
                kWallLineH,
                ResearchLayoutElement::WallLine,
                true
            );
        }
        break;
    case 3:
        for (int i = show_start_id_; i <= show_end_id_; ++i) {
            bow_list_[static_cast<size_t>(i)]->draw(origin_x_, origin_y_, show_x_);
        }
        break;
    default:
        break;
    }

    draw_research_layout_texture_region(
        kItemCover,
        0.0f,
        0.0f,
        800.0f,
        480.0f,
        kItemCoverCropX,
        kItemCoverCropY,
        kItemCoverCropW,
        kItemCoverCropH,
        ResearchLayoutElement::EquipCover
    );
    des_.draw();
    up_area_.draw();
}

void ItemZone::update() {
    if (!move_flag_ && move_speed_ != 0.0f) {
        const float sign = std::abs(move_speed_) / move_speed_;
        move_speed_ -= ((sign * 1500.0f) * static_cast<float>(AbstractGame::last_span_ms())) / 1000.0f;
        add_show_x((move_speed_ * static_cast<float>(AbstractGame::last_span_ms())) / 1000.0f);
        if (std::abs(move_speed_) < 100.0f) {
            move_speed_ = 0.0f;
        }
    }
    HelpOverlay::update();
}

} // namespace defender
