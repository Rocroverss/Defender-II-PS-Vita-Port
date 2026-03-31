#include "game/xp_data.hpp"

namespace defender {

int XpData::get_max_xp(int lv) {
    int temp = lv / 10;
    int result = (temp + 1) * (lv % 10) * 100;
    while (temp > 0) {
        result += temp * 1000;
        if (lv > 50) {
            result += 1000;
        }
        --temp;
    }
    return result;
}

} // namespace defender

