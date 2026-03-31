#pragma once

namespace defender {

struct Status {
    static constexpr int DIE       = 0;
    static constexpr int RUN       = 1;
    static constexpr int ATTACK    = 2;
    static constexpr int REMOVE    = 3;
    static constexpr int WAIT      = 4;
    static constexpr int VISITABLE = 10;
    static constexpr int GONE      = 11;
};

} // namespace defender

