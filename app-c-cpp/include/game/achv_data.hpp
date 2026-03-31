#pragma once

#include <array>
#include <string>

#include "game/param.hpp"
#include "game/save.hpp"

namespace defender {

class AchvData {
public:
    static constexpr int GOLD = 0;
    static constexpr int STONE = 1;
    static constexpr int KILL = 2;
    static constexpr int STAGE = 3;
    static constexpr int WIN = 4;
    static constexpr int FIRE = 5;
    static constexpr int ICE = 6;
    static constexpr int LIGHT = 7;
    static constexpr int ACHV_TOTAL_NUM = 8;

    static int get_cur_amount(int type) {
        switch (type) {
        case GOLD:
            return Param::cost_coin;
        case STONE:
            return Param::cost_stone;
        case KILL:
            return Param::total_kills;
        case STAGE:
            return Param::stage;
        case WIN:
            return Param::win;
        case FIRE:
            return Param::cast_fire;
        case ICE:
            return Param::cast_ice;
        case LIGHT:
            return Param::cast_light;
        default:
            return 0;
        }
    }

    static int get_level(int type, int amount) {
        if (type < 0 || type >= ACHV_TOTAL_NUM) {
            return 0;
        }
        int level = 0;
        const auto& row = amount_list()[static_cast<size_t>(type)];
        for (int i = 0; i < 3; ++i) {
            if (amount >= row[static_cast<size_t>(i)]) {
                level = i + 1;
            }
        }
        return level;
    }

    static int get_reward(int type, int level) {
        if (type < 0 || type >= ACHV_TOTAL_NUM || level <= 0) {
            return 0;
        }
        return reward_list()[static_cast<size_t>(type)][static_cast<size_t>(level - 1)];
    }

    static int get_max_amount(int type, int level) {
        if (type < 0 || type >= ACHV_TOTAL_NUM) {
            return 0;
        }
        if (level > 2) {
            level = 2;
        } else if (level < 0) {
            level = 0;
        }
        return amount_list()[static_cast<size_t>(type)][static_cast<size_t>(level)];
    }

    static void save_achv_data() {
        Save::save_data(Save::FIRE_CAST, Param::cast_fire);
        Save::save_data(Save::ICE_CAST, Param::cast_ice);
        Save::save_data(Save::LIGHT_CAST, Param::cast_light);
        Save::save_data(Save::COST_COIN, Param::cost_coin);
        Save::save_data(Save::COST_STONE, Param::cost_stone);
        Save::save_data(Save::KILLS, Param::total_kills);
    }

private:
    static const std::array<std::array<int, 3>, ACHV_TOTAL_NUM>& amount_list() {
        static constexpr std::array<std::array<int, 3>, ACHV_TOTAL_NUM> kAmountList = {{
            {{50000, 250000, 1000000}},
            {{60, 300, 1000}},
            {{5000, 30000, 200000}},
            {{50, 150, 350}},
            {{30, 100, 300}},
            {{200, 1000, 3000}},
            {{200, 1000, 3000}},
            {{200, 1000, 3000}}
        }};
        return kAmountList;
    }

    static const std::array<std::array<int, 3>, ACHV_TOTAL_NUM>& reward_list() {
        static constexpr std::array<std::array<int, 3>, ACHV_TOTAL_NUM> kRewardList = {{
            {{1, 2, 3}},
            {{10, 15, 20}},
            {{5, 10, 20}},
            {{5, 15, 30}},
            {{5, 10, 20}},
            {{10, 20, 30}},
            {{10, 20, 30}},
            {{10, 20, 30}}
        }};
        return kRewardList;
    }
};

} // namespace defender
