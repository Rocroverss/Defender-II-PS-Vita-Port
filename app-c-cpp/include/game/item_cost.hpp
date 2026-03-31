#pragma once

namespace defender {

class ItemCost {
public:
    static bool is_gold_pay(int type);
    static int get_cost(int type);
};

} // namespace defender

