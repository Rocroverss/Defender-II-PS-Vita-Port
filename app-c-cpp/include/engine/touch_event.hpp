#pragma once

namespace defender {

enum class TouchAction {
    None,
    Down,
    Up,
    Move
};

struct TouchEvent {
    TouchAction action = TouchAction::None;
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
    int pointer_count = 0;
};

}

