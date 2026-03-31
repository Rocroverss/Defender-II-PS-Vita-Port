#pragma once

#include "scenes/scene.hpp"

namespace defender {

class SolidScene : public Scene {
public:
    SolidScene(float r, float g, float b);

    void draw() override;
    bool touch(const TouchEvent& event) override;
    void update() override;
    void reset() override;

private:
    float r_;
    float g_;
    float b_;
    float pulse_ = 0.0f;
    bool touched_ = false;
};

}

