#include "game/item_param.hpp"

#include <array>
#include <cstddef>
#include <string>

#include "game/save.hpp"
#include "game/skill_data.hpp"

namespace defender {
namespace {

std::array<int, ItemParam::TOTAL_SIZE> s_self_level = {};
std::array<int, ItemParam::TOTAL_SIZE> s_replay_level = {};

} // namespace

void ItemParam::init_level(int type, int level) {
    init_level(type, level, true);
}

void ItemParam::init_level(int type, int level, bool is_self_level) {
    if (type < 0 || type >= TOTAL_SIZE) {
        return;
    }
    if (is_self_level) {
        s_self_level[static_cast<size_t>(type)] = level;
        if (type < LEVEL_SIZE) {
            SkillData::init_level(type, level);
        }
    } else {
        s_replay_level[static_cast<size_t>(type)] = level;
    }
}

int ItemParam::get_level(int type) {
    return get_level(type, true);
}

int ItemParam::get_level(int type, bool is_self_level) {
    if (type < 0 || type >= TOTAL_SIZE) {
        return 0;
    }
    return is_self_level ? s_self_level[static_cast<size_t>(type)] : s_replay_level[static_cast<size_t>(type)];
}

void ItemParam::load_level() {
    init_level(BOW_EQUIP, Save::load_data(Save::EQUIPED_BOW));
    init_level(MAGIC1_EQUIP, Save::load_data(std::string(Save::EQUIPED_MAGIC) + "0"));
    init_level(MAGIC2_EQUIP, Save::load_data(std::string(Save::EQUIPED_MAGIC) + "1"));
    init_level(MAGIC3_EQUIP, Save::load_data(std::string(Save::EQUIPED_MAGIC) + "2"));
    for (int i = 0; i < LEVEL_SIZE; ++i) {
        init_level(i, Save::load_data(std::string(Save::SKILL_LEVEL) + std::to_string(i)));
    }
}

} // namespace defender
