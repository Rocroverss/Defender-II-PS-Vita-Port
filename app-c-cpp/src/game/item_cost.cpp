#include "game/item_cost.hpp"

#include "game/item_param.hpp"

namespace defender {
namespace {

constexpr int kStrCost[2] = {300, 200};
constexpr int kAgiCost[2] = {300, 200};
constexpr int kPowShotCost[2] = {1000, 800};
constexpr int kFatalBlowCost[2] = {1500, 1500};
constexpr int kMultiArrowCost[2] = {5000, 10000};
constexpr int kSeniorHunterCost[2] = {2000, 2000};
constexpr int kPoisonArrowCost[2] = {3000, 4000};
constexpr int kManaResearchCost[2] = {5, 5};
constexpr int kMagic1Cost[2] = {5, 2};
constexpr int kMagic2Cost[2] = {9, 3};
constexpr int kMagic3Cost[2] = {13, 4};
constexpr int kCityWallCost[2] = {700, 800};
constexpr int kMagicTowerCost[2] = {10, 140};
constexpr int kTowerAtkCost[2] = {1000, 500};
constexpr int kTowerSplCost[2] = {10, 5};
constexpr int kLavaCost[2] = {10, 140};
constexpr int kLavaAtkCost[2] = {1000, 500};
constexpr int kLavaSlowCost[2] = {10, 5};

} // namespace

bool ItemCost::is_gold_pay(int type) {
    switch (type) {
    case 1:
    case 3:
    case 4:
    case 6:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
        return false;
    default:
        return true;
    }
}

int ItemCost::get_cost(int type) {
    const int level = ItemParam::get_level(type);
    switch (type) {
    case 0:
        return kCityWallCost[0] + (kCityWallCost[1] * level);
    case 1:
        return kLavaCost[0] + (kLavaCost[1] * level);
    case 2:
        return kLavaAtkCost[0] + (kLavaAtkCost[1] * level);
    case 3:
        return kLavaSlowCost[0] + (kLavaSlowCost[1] * level);
    case 4:
        return kMagicTowerCost[0] + (kMagicTowerCost[1] * level);
    case 5:
        return kTowerAtkCost[0] + (kTowerAtkCost[1] * level);
    case 6:
        return kTowerSplCost[0] + (kTowerSplCost[1] * level);
    case 7: {
        int cost = kStrCost[0] + (kStrCost[1] * level);
        if (level >= 90) {
            cost += (level - 89) * 20 * kStrCost[1];
        }
        return cost;
    }
    case 8: {
        int cost = kAgiCost[0] + (kAgiCost[1] * level);
        if (level >= 30) {
            cost += (level - 29) * 2500;
        }
        if (level >= 35) {
            cost += (level - 34) * 20000;
        }
        return cost;
    }
    case 9: {
        int cost = kPowShotCost[0] + (kPowShotCost[1] * level);
        if (level >= 5) {
            cost += (level - 4) * 10 * kPowShotCost[1];
        }
        return cost;
    }
    case 10: {
        int cost = kFatalBlowCost[0] + (kFatalBlowCost[1] * level);
        if (level >= 5) {
            cost += (level - 4) * 10 * kFatalBlowCost[1];
        }
        return cost;
    }
    case 11: {
        int cost = kMultiArrowCost[0] + (kMultiArrowCost[1] * level);
        if (level >= 5) {
            cost += (level - 4) * 2 * kMultiArrowCost[1];
        }
        if (level > 9) {
            cost += (level - 9) * 2 * kMultiArrowCost[1];
        }
        return cost;
    }
    case 12:
        return kSeniorHunterCost[0] + (kSeniorHunterCost[1] * level);
    case 13:
        return kPoisonArrowCost[0] + (kPoisonArrowCost[1] * level);
    case 14:
        return kManaResearchCost[0] + (kManaResearchCost[1] * level);
    case 15:
    case 18:
    case 21:
        return kMagic1Cost[0] + (kMagic1Cost[1] * level);
    case 16:
    case 19:
    case 22:
        return kMagic2Cost[0] + (kMagic2Cost[1] * level);
    case 17:
    case 20:
    case 23: {
        const int cost = kMagic3Cost[0] + (kMagic3Cost[1] * level);
        return cost > 99 ? 99 : cost;
    }
    default:
        return 0;
    }
}

} // namespace defender

