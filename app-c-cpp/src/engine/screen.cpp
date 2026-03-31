#include "engine/screen.hpp"

#include "engine/bmp_scaler.hpp"
#include "engine/coordinate_mapper.hpp"

namespace defender {

Screen& Screen::current() {
    static Screen screen;
    return screen;
}

bool Screen::set_bounds(int width, int height) {
    bool changed = false;
    if (width_ != width) {
        width_ = width;
        changed = true;
    }
    if (height_ != height) {
        height_ = height;
        changed = true;
    }

    if (changed) {
        BmpScaler::instance().update_scale();
        CoordinateMapper::instance().update_scale();
    }

    return changed;
}

}

