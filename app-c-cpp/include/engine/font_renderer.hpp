#pragma once

#include <string>

namespace defender {

enum class FontFaceId {
    Ants,
    Cooper
};

class FontRenderer {
public:
    static FontRenderer& instance();

    bool available() const;
    void clear();

    // Draw text using baseline coordinates in designed-space units.
    void draw_text(
        FontFaceId font,
        const std::string& text,
        float baseline_x,
        float baseline_y,
        float pixel_height,
        float r = 1.0f,
        float g = 1.0f,
        float b = 1.0f,
        float a = 1.0f
    );

    float measure_text_width(
        FontFaceId font,
        const std::string& text,
        float pixel_height
    );

private:
    FontRenderer();
    ~FontRenderer();
    FontRenderer(const FontRenderer&) = delete;
    FontRenderer& operator=(const FontRenderer&) = delete;

    struct Impl;
    Impl* impl_ = nullptr;
};

} // namespace defender
