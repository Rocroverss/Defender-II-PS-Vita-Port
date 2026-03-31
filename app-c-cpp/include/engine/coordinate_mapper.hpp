#pragma once

namespace defender {

class CoordinateMapper {
public:
    static CoordinateMapper& instance();

    void set_designed(float designed_width, float designed_height);
    void update_scale();

    float gen_game_length(float length_in_designed_screen) const;
    float gen_game_length_x(float length_in_designed_screen) const;
    float gen_game_length_y(float length_in_designed_screen) const;

private:
    float designed_width_ = 800.0f;
    float designed_height_ = 480.0f;
    bool inited_ = false;
    float scale_x_ = 1.0f;
    float scale_y_ = 1.0f;
    float scale_min_ = 1.0f;
};

}

