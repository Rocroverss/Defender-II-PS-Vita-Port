#pragma once

namespace defender {

enum class ScaleType {
    FitScreen,
    KeepRatio
};

class BmpScaler {
public:
    static BmpScaler& instance();

    void set_bitmap_original(float width, float height);
    void update_scale();

    float scale_x(float width, ScaleType scale_type) const;
    float scale_y(float height, ScaleType scale_type) const;

private:
    float original_width_ = 800.0f;
    float original_height_ = 480.0f;
    bool inited_ = false;
    float scale_x_ = 1.0f;
    float scale_y_ = 1.0f;
    float scale_min_ = 1.0f;
};

}

