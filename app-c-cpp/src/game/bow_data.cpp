#include "game/bow_data.hpp"

#include <array>
#include <cstddef>

#include "game/param.hpp"

namespace defender {
namespace {

constexpr std::array<int, 29> kBowCostList = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    500, 1000, 2000, 4000, 7000, 11000, 17000, 23000, 30000,
    2, 5, 9, 15, 26, 39, 58, 80, 100, 250
};

constexpr std::array<std::array<int, 5>, 29> kBowDataList = {{
    {{0, 0, 0, 0, 0}},
    {{1, 0, 0, 0, 0}},
    {{2, 0, 0, 0, 0}},
    {{3, 0, 1, 0, 0}},
    {{4, 0, 1, 0, 0}},
    {{5, 0, 1, 0, 0}},
    {{6, 0, 2, 0, 0}},
    {{7, 0, 2, 0, 0}},
    {{8, 0, 2, 0, 0}},
    {{9, 0, 3, 0, 0}},
    {{1, 1, 0, 0, 0}},
    {{2, 2, 0, 0, 0}},
    {{3, 2, 0, 1, 0}},
    {{4, 3, 0, 1, 0}},
    {{4, 4, 0, 2, 0}},
    {{5, 5, 0, 2, 0}},
    {{6, 5, 0, 3, 0}},
    {{7, 6, 0, 3, 0}},
    {{8, 6, 0, 4, 0}},
    {{2, 1, 0, 0, 0}},
    {{3, 2, 0, 1, 0}},
    {{4, 3, 1, 1, 0}},
    {{5, 4, 1, 2, 0}},
    {{6, 5, 1, 2, 1}},
    {{7, 6, 2, 2, 1}},
    {{8, 6, 2, 3, 2}},
    {{9, 7, 3, 3, 2}},
    {{9, 8, 3, 4, 3}},
    {{15, 9, 4, 5, 4}}
}};

constexpr std::array<std::array<int, 5>, 20> kFinalData = {{
    {{2, 2, 0, 0, 0}},
    {{2, 2, 0, 1, 0}},
    {{3, 3, 0, 1, 0}},
    {{4, 4, 1, 1, 0}},
    {{5, 5, 1, 2, 0}},
    {{6, 5, 1, 2, 1}},
    {{7, 6, 2, 2, 1}},
    {{8, 7, 2, 3, 1}},
    {{9, 7, 2, 3, 2}},
    {{10, 8, 2, 4, 2}},
    {{11, 8, 3, 4, 2}},
    {{12, 8, 3, 5, 3}},
    {{13, 9, 3, 5, 3}},
    {{14, 9, 4, 5, 3}},
    {{15, 9, 4, 5, 4}},
    {{18, 9, 4, 5, 4}},
    {{21, 9, 4, 5, 4}},
    {{24, 9, 4, 5, 4}},
    {{27, 9, 4, 5, 4}},
    {{30, 9, 4, 5, 4}}
}};

} // namespace

int BowData::get_cost(int bow_id) {
    if (bow_id < 0 || bow_id >= static_cast<int>(kBowCostList.size())) {
        return 0;
    }
    return kBowCostList[static_cast<size_t>(bow_id)];
}

int BowData::get_ability(int bow_id, int type) {
    if (type < 0 || type >= 5) {
        return 0;
    }
    if (bow_id < 0 || bow_id >= static_cast<int>(kBowDataList.size())) {
        return 0;
    }
    if (bow_id != FINAL) {
        return kBowDataList[static_cast<size_t>(bow_id)][static_cast<size_t>(type)];
    }
    int temp = Param::level / 5;
    if (temp > 19) {
        temp = 19;
    }
    if (temp < 0) {
        temp = 0;
    }
    return kFinalData[static_cast<size_t>(temp)][static_cast<size_t>(type)];
}

} // namespace defender
