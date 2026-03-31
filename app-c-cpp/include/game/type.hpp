#pragma once

namespace defender {

struct Type {
    static constexpr int NORMAL         = 0;
    static constexpr int MAGIC          = 1;
    static constexpr int BOW            = 2;

    static constexpr int IGNORE_DEFENCE = 0;
    static constexpr int INC_PER_LV     = 1;
    static constexpr int BUY_FEE        = 2;
    static constexpr int LIMIT_LEVEL    = 3;

    static constexpr int ORIGINAL       = 0;
    static constexpr int FIRE           = 1;
    static constexpr int ICE            = 2;
    static constexpr int LIGHT          = 3;
};

} // namespace defender

