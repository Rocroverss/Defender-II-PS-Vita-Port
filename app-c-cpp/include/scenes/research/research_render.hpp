#pragma once

#include <string>

#include "engine/sprite_atlas_cache.hpp"

namespace defender {

class Scene;

struct RectF {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    bool contains(float x, float y) const {
        return x >= left && x <= right && y >= top && y <= bottom;
    }
};

float sx(float x);
float sy(float y);
float sxy(float xy);

RectF make_rect(float x, float y, float w, float h);
bool hit_box(float x, float y, float w, float h, float px, float py);

bool draw_texture(const std::string& path, float x, float y, float w = -1.0f, float h = -1.0f, float alpha = 1.0f);
bool draw_texture_logical(const std::string& path, float x, float y, float w = -1.0f, float h = -1.0f, float alpha = 1.0f);
bool draw_texture_logical_region(
    const std::string& path,
    float x,
    float y,
    float w,
    float h,
    float src_x,
    float src_y,
    float src_w,
    float src_h,
    float alpha = 1.0f
);
bool draw_texture_tinted(
    const std::string& path,
    float x,
    float y,
    float w,
    float h,
    float r,
    float g,
    float b,
    float a
);

const AtlasFrame* get_research_frame(const std::string& frame_name);
bool draw_frame(const std::string& frame_name, float x, float y, float scale = 1.0f, float alpha = 1.0f);
bool draw_frame_tinted(
    const std::string& frame_name,
    float x,
    float y,
    float scale,
    float r,
    float g,
    float b,
    float a
);

float frame_width(const std::string& frame_name, float scale = 1.0f, float fallback = 0.0f);
float frame_height(const std::string& frame_name, float scale = 1.0f, float fallback = 0.0f);

float draw_number_strip(
    const std::string& texture_path,
    int value,
    float x,
    float y,
    float scale = 1.0f,
    float r = 1.0f,
    float g = 1.0f,
    float b = 1.0f,
    float a = 1.0f
);

} // namespace defender
