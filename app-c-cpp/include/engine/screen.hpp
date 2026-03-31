#pragma once

namespace defender {

class Screen {
public:
    static Screen& current();

    bool set_bounds(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

private:
    int width_ = 960;
    int height_ = 544;
};

}

