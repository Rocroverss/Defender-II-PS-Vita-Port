#pragma once

#include <array>
#include <cstddef>

namespace defender {

class SkillData {
public:
    static constexpr int WALL          = 0;
    static constexpr int RIVER         = 1;
    static constexpr int RIVER_ATK     = 2;
    static constexpr int RIVER_SLOW    = 3;
    static constexpr int TOWER         = 4;
    static constexpr int TOWER_ATK     = 5;
    static constexpr int TOWER_SPU     = 6;
    static constexpr int STR           = 7;
    static constexpr int AGI           = 8;
    static constexpr int POWER_SHOT    = 9;
    static constexpr int FATAL_BLOW    = 10;
    static constexpr int MULTI_ARROW   = 11;
    static constexpr int SENIOR_HUNTER = 12;
    static constexpr int POISON_ARROW  = 13;
    static constexpr int MANA_BASIC    = 14;

    static constexpr std::size_t LEVEL_SIZE = 28;

    static int get_value(int type);
    static int get_value(int type, int add_level);

    static void init_level(int type, int level);
    static int get_level(int type);
    static void reset_levels();

private:
    static std::array<int, LEVEL_SIZE> levels_;
};

} // namespace defender

