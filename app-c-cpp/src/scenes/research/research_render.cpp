#include "scenes/research/research_render.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>

#include <vitaGL.h>

#include "engine/game_texture_specs.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "scenes/scene.hpp"

namespace defender {
namespace {

void draw_uv_tinted(
    GLuint texture,
    float x,
    float y,
    float w,
    float h,
    float u0,
    float v0,
    float u1,
    float v1,
    float r,
    float g,
    float b,
    float a
) {
    if (texture == 0 || w <= 0.0f || h <= 0.0f || a <= 0.0f) {
        return;
    }

    const GLfloat vertices[] = {
        x, y, 0.0f,
        x + w, y, 0.0f,
        x, y + h, 0.0f,
        x + w, y + h, 0.0f
    };
    const GLfloat uvs[] = {
        u0, v0,
        u1, v0,
        u0, v1,
        u1, v1
    };

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glColor4f(r, g, b, a);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

} // namespace

float sx(float x) {
    return Scene::get_x(x);
}

float sy(float y) {
    return Scene::get_y(y);
}

float sxy(float xy) {
    return Scene::get_xy(xy);
}

RectF make_rect(float x, float y, float w, float h) {
    RectF rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + w;
    rect.bottom = y + h;
    return rect;
}

bool hit_box(float x, float y, float w, float h, float px, float py) {
    const RectF rect = make_rect(sx(x), sy(y), sx(w), sy(h));
    return rect.contains(px, py);
}

bool draw_texture(const std::string& path, float x, float y, float w, float h, float alpha) {
    const auto& tex = TextureCache::instance().get(path);
    if (!tex.valid || tex.id == 0 || alpha <= 0.0f) {
        return false;
    }
    const float draw_w = w > 0.0f ? w : static_cast<float>(tex.width);
    const float draw_h = h > 0.0f ? h : static_cast<float>(tex.height);
    draw_textured_quad(tex.id, x, y, draw_w, draw_h, alpha);
    return true;
}

bool draw_texture_logical(const std::string& path, float x, float y, float w, float h, float alpha) {
    const auto& tex = TextureCache::instance().get(path);
    if (!tex.valid || tex.id == 0 || alpha <= 0.0f) {
        return false;
    }

    const size_t slash = path.find_last_of("/\\");
    const std::string_view file_name = slash == std::string::npos
        ? std::string_view(path)
        : std::string_view(path).substr(slash + 1);
    const float draw_w = w > 0.0f ? w : static_cast<float>(game_texture_width_px(tex, file_name));
    const float draw_h = h > 0.0f ? h : static_cast<float>(game_texture_height_px(tex, file_name));
    return draw_game_texture_quad(tex, file_name, x, y, draw_w, draw_h, alpha);
}

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
    float alpha
) {
    const auto& tex = TextureCache::instance().get(path);
    if (!tex.valid || tex.id == 0 || alpha <= 0.0f || w <= 0.0f || h <= 0.0f || src_w <= 0.0f || src_h <= 0.0f) {
        return false;
    }

    const size_t slash = path.find_last_of("/\\");
    const std::string_view file_name = slash == std::string::npos
        ? std::string_view(path)
        : std::string_view(path).substr(slash + 1);
    const float logical_w = static_cast<float>(game_texture_width_px(tex, file_name));
    const float logical_h = static_cast<float>(game_texture_height_px(tex, file_name));
    if (logical_w <= 0.0f || logical_h <= 0.0f) {
        return false;
    }

    const float logical_u1 = game_texture_u1(tex, file_name);
    const float logical_v0 = game_texture_v0(tex, file_name);
    const float u0 = logical_u1 * (src_x / logical_w);
    const float u1 = logical_u1 * ((src_x + src_w) / logical_w);
    const float v0 = logical_v0 * (1.0f - (src_y / logical_h));
    const float v1 = logical_v0 * (1.0f - ((src_y + src_h) / logical_h));
    draw_textured_quad_uv(tex.id, x, y, w, h, u0, v0, u1, v1, alpha);
    return true;
}

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
) {
    const auto& tex = TextureCache::instance().get(path);
    if (!tex.valid || tex.id == 0 || a <= 0.0f) {
        return false;
    }
    const float draw_w = w > 0.0f ? w : static_cast<float>(tex.width);
    const float draw_h = h > 0.0f ? h : static_cast<float>(tex.height);
    draw_textured_quad_tinted(tex.id, x, y, draw_w, draw_h, r, g, b, a);
    return true;
}

const AtlasFrame* get_research_frame(const std::string& frame_name) {
    return SpriteAtlasCache::instance().get_frame(frame_name);
}

bool draw_frame(const std::string& frame_name, float x, float y, float scale, float alpha) {
    return draw_frame_tinted(frame_name, x, y, scale, 1.0f, 1.0f, 1.0f, alpha);
}

bool draw_frame_tinted(
    const std::string& frame_name,
    float x,
    float y,
    float scale,
    float r,
    float g,
    float b,
    float a
) {
    const AtlasFrame* frame = get_research_frame(frame_name);
    if (frame == nullptr || !frame->valid || frame->texture_id == 0 || scale <= 0.0f || a <= 0.0f) {
        return false;
    }

    const float src_w = static_cast<float>(std::max(1, frame->original_width));
    const float src_h = static_cast<float>(std::max(1, frame->original_height));
    const float target_w = src_w * scale;
    const float target_h = src_h * scale;
    const float draw_w = static_cast<float>(frame->width) * scale;
    const float draw_h = static_cast<float>(frame->height) * scale;
    const float draw_x = x + ((target_w - draw_w) * 0.5f) + (frame->offset_x * scale);
    const float draw_y = y + ((target_h - draw_h) * 0.5f) + (frame->offset_y * scale);

    draw_uv_tinted(
        frame->texture_id,
        draw_x,
        draw_y,
        draw_w,
        draw_h,
        frame->u0,
        frame->v0,
        frame->u1,
        frame->v1,
        r,
        g,
        b,
        a
    );
    return true;
}

float frame_width(const std::string& frame_name, float scale, float fallback) {
    const AtlasFrame* frame = get_research_frame(frame_name);
    if (frame == nullptr || !frame->valid) {
        return fallback;
    }
    return static_cast<float>(std::max(1, frame->original_width)) * scale;
}

float frame_height(const std::string& frame_name, float scale, float fallback) {
    const AtlasFrame* frame = get_research_frame(frame_name);
    if (frame == nullptr || !frame->valid) {
        return fallback;
    }
    return static_cast<float>(std::max(1, frame->original_height)) * scale;
}

float draw_number_strip(
    const std::string& texture_path,
    int value,
    float x,
    float y,
    float scale,
    float r,
    float g,
    float b,
    float a
) {
    const auto& tex = TextureCache::instance().get(texture_path);
    if (!tex.valid || tex.id == 0 || scale <= 0.0f || a <= 0.0f) {
        return 0.0f;
    }

    int safe_value = value;
    if (safe_value < 0) {
        safe_value = 0;
    }
    const std::string text = std::to_string(safe_value);
    const size_t slash = texture_path.find_last_of("/\\");
    const std::string file_name = slash == std::string::npos
        ? texture_path
        : texture_path.substr(slash + 1);

    const float logical_u1 = game_texture_u1(tex, file_name);
    const float logical_v0 = game_texture_v0(tex, file_name);
    const float cell_w = static_cast<float>(game_texture_width_px(tex, file_name)) / 10.0f;
    const float cell_h = static_cast<float>(game_texture_height_px(tex, file_name));
    const float draw_w = cell_w * scale;
    const float draw_h = cell_h * scale;

    float pen_x = x;
    for (char ch : text) {
        const int digit = std::clamp(static_cast<int>(ch - '0'), 0, 9);
        const float u0 = logical_u1 * (static_cast<float>(digit) / 10.0f);
        const float u1 = logical_u1 * (static_cast<float>(digit + 1) / 10.0f);
        draw_uv_tinted(tex.id, pen_x, y, draw_w, draw_h, u0, logical_v0, u1, 0.0f, r, g, b, a);
        pen_x += draw_w;
    }
    return static_cast<float>(text.size()) * draw_w;
}

} // namespace defender
