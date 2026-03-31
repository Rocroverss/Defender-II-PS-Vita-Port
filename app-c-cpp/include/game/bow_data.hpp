#pragma once

namespace defender {

class BowData {
public:
    static constexpr int NORMAL = 0;
    static constexpr int VOL = 1;
    static constexpr int HUR = 10;
    static constexpr int PHA = 19;
    static constexpr int FINAL = 28;

    static int get_cost(int bow_id);
    static int get_ability(int bow_id, int type);
};

} // namespace defender

