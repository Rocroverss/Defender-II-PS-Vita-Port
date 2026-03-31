#include "game/skill_data.hpp"

namespace defender {
namespace {

constexpr int kWallValue[]      = {90, 10, 0, 21};
constexpr int kRiverValue[]     = {0, 1, 0, 3};
constexpr int kRiverAtkValue[]  = {20, 5, 0, 30};
constexpr int kRiverSlowValue[] = {10, 2, 0, 10};
constexpr int kTowerValue[]     = {0, 1, 0, 2};
constexpr int kTowerAtkValue[]  = {40, 5, 0, 30};
constexpr int kTowerSplValue[]  = {5, 5, 0, 10};
constexpr int kStrValue[]       = {20, 4, 0, 90};
constexpr int kAgiValue[]       = {15, 2, 0, 30};
constexpr int kPsValue[]        = {3, 5, 0, 0};
constexpr int kFbValue[]        = {5, 5, 0, 0};
constexpr int kMaValue[] = {
    55, 60, 65, 70, 50, 55, 60, 65, 50, 60, 65, 70, 60, 65, 70, 75
};
constexpr int kShValue[]   = {0, 5, 0, 20};
constexpr int kPaValue[]   = {0, 5, 0, 10};
constexpr int kManaValue[] = {80, 20, 0, 11};

} // namespace

std::array<int, SkillData::LEVEL_SIZE> SkillData::levels_ = {};

int SkillData::get_value(int type) {
    return get_value(type, 0);
}

int SkillData::get_value(int type, int add_level) {
    const int level = get_level(type) + add_level;
    switch (type) {
    case WALL:
        return kWallValue[0] + (level * kWallValue[1]);
    case RIVER:
        return kRiverValue[0] + (level * kRiverValue[1]);
    case RIVER_ATK:
        return kRiverAtkValue[0] + (level * kRiverAtkValue[1]);
    case RIVER_SLOW:
        if (level != 0) {
            return kRiverSlowValue[0] + (level * kRiverSlowValue[1]);
        }
        return 0;
    case TOWER:
        return kTowerValue[0] + (level * kTowerValue[1]);
    case TOWER_ATK:
        return kTowerAtkValue[0] + (level * kTowerAtkValue[1]);
    case TOWER_SPU:
        if (level != 0) {
            return kTowerSplValue[0] + (level * kTowerSplValue[1]);
        }
        return 0;
    case STR:
        return kStrValue[0] + (level * kStrValue[1]);
    case AGI:
        return kAgiValue[0] + (level * kAgiValue[1]);
    case POWER_SHOT:
        return kPsValue[0] + (level * kPsValue[1]);
    case FATAL_BLOW:
        if (level == 0) {
            return 0;
        }
        if (level > 14) {
            return (level + 75) - 14;
        }
        return kFbValue[0] + (kFbValue[1] * level);
    case MULTI_ARROW:
        if (level == 0) {
            return 100;
        }
        if (level > static_cast<int>(sizeof(kMaValue) / sizeof(kMaValue[0]))) {
            return ((level - static_cast<int>(sizeof(kMaValue) / sizeof(kMaValue[0]))) * 2) + 75;
        }
        return kMaValue[level - 1];
    case SENIOR_HUNTER:
        return kShValue[0] + (level * kShValue[1]);
    case POISON_ARROW:
        return kPaValue[0] + (level * kPaValue[1]);
    case MANA_BASIC: {
        const int temp_value = kManaValue[0] + (kManaValue[1] * level);
        return level > 11 ? temp_value - (((level - 11) * kManaValue[1]) / 2) : temp_value;
    }
    default:
        return -1;
    }
}

void SkillData::init_level(int type, int level) {
    if (type < 0 || type >= static_cast<int>(levels_.size())) {
        return;
    }
    levels_[static_cast<std::size_t>(type)] = level;
}

int SkillData::get_level(int type) {
    if (type < 0 || type >= static_cast<int>(levels_.size())) {
        return 0;
    }
    return levels_[static_cast<std::size_t>(type)];
}

void SkillData::reset_levels() {
    levels_.fill(0);
}

} // namespace defender

