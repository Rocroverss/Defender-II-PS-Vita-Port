#pragma once

namespace defender {

class ItemParam {
public:
    static constexpr int WALL = 0;
    static constexpr int RIVER = 1;
    static constexpr int RIVER_ATK = 2;
    static constexpr int RIVER_SLOW = 3;
    static constexpr int TOWER = 4;
    static constexpr int TOWER_ATK = 5;
    static constexpr int TOWER_SPU = 6;
    static constexpr int STR = 7;
    static constexpr int AGI = 8;
    static constexpr int POWER_SHOT = 9;
    static constexpr int FATAL_BLOW = 10;
    static constexpr int MULTI_ARROW = 11;
    static constexpr int SENIOR_HUNTER = 12;
    static constexpr int POISON_ARROW = 13;
    static constexpr int MANA_RESEARCH = 14;
    static constexpr int FIRE_1 = 15;
    static constexpr int FIRE_2 = 16;
    static constexpr int FIRE_3 = 17;
    static constexpr int ICE_1 = 18;
    static constexpr int ICE_2 = 19;
    static constexpr int ICE_3 = 20;
    static constexpr int LIGHT_1 = 21;
    static constexpr int LIGHT_2 = 22;
    static constexpr int LIGHT_3 = 23;
    static constexpr int BOW_EQUIP = 24;
    static constexpr int MAGIC1_EQUIP = 25;
    static constexpr int MAGIC2_EQUIP = 26;
    static constexpr int MAGIC3_EQUIP = 27;

    static constexpr int LEVEL_SIZE = 24;
    static constexpr int EQUIP_SIZE = 4;
    static constexpr int TOTAL_SIZE = 28;

    static void init_level(int type, int level);
    static void init_level(int type, int level, bool is_self_level);
    static int get_level(int type);
    static int get_level(int type, bool is_self_level);
    static void load_level();
};

} // namespace defender

