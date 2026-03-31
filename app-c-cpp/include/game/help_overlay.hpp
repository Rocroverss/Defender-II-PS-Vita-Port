#pragma once

#include "engine/touch_event.hpp"

namespace defender {

class HelpOverlay {
public:
    static void init();
    static void reset_runtime();

    static bool is_show();
    static void active();
    static void set_help(int type);

    // Returns true when the overlay consumes the touch.
    static bool touch(const TouchEvent& event, float x, float y);
    static void draw();
    static void update(bool allow_battle_script = false);
};

} // namespace defender
