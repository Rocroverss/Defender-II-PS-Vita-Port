#pragma once

#include <vitaGL.h>

#include "engine/coordinate_mapper.hpp"
#include "engine/touch_event.hpp"

namespace defender {

class Scene {
public:
    virtual ~Scene() = default;

    virtual void draw() = 0;
    virtual bool touch(const TouchEvent& event) = 0;
    virtual void update() = 0;
    virtual void reset() {}

    void scene_draw();

    static void scene_init();
    static float get_xy(float xy);
    static float get_x(float x);
    static float get_y(float y);

    float scene_x = 0.0f;
    float scene_y = 0.0f;
    float scene_scale = 1.0f;
    float scene_alpha = 1.0f;

    static float screen_width;
    static float screen_height;
};

}

