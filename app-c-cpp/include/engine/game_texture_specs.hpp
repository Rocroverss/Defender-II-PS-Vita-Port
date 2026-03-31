#pragma once

#include <array>
#include <string_view>
#include <utility>

#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"

namespace defender {

struct GameTextureSpec {
    int width;
    int height;
    int wrap_width;
    int wrap_height;
};

inline bool parse_numbered_png(std::string_view name,
                               std::string_view prefix,
                               int* out_index) {
    if (out_index == nullptr) {
        return false;
    }
    constexpr std::string_view kSuffix = ".png";
    if (name.size() != prefix.size() + 2 + kSuffix.size()) {
        return false;
    }
    if (name.compare(0, prefix.size(), prefix) != 0) {
        return false;
    }
    if (name.compare(name.size() - kSuffix.size(), kSuffix.size(), kSuffix) != 0) {
        return false;
    }
    const char tens = name[prefix.size()];
    const char ones = name[prefix.size() + 1];
    if (tens < '0' || tens > '9' || ones < '0' || ones > '9') {
        return false;
    }
    const int value = ((tens - '0') * 10) + (ones - '0');
    if (value < 1 || value > 9) {
        return false;
    }
    *out_index = value - 1;
    return true;
}

inline bool parse_numbered_png_3(std::string_view name,
                                 std::string_view prefix,
                                 int* out_index,
                                 int max_value) {
    if (out_index == nullptr) {
        return false;
    }
    constexpr std::string_view kSuffix = ".png";
    if (name.size() != prefix.size() + 3 + kSuffix.size()) {
        return false;
    }
    if (name.compare(0, prefix.size(), prefix) != 0) {
        return false;
    }
    if (name.compare(name.size() - kSuffix.size(), kSuffix.size(), kSuffix) != 0) {
        return false;
    }
    const char hundreds = name[prefix.size()];
    const char tens = name[prefix.size() + 1];
    const char ones = name[prefix.size() + 2];
    if (hundreds < '0' || hundreds > '9' ||
        tens < '0' || tens > '9' ||
        ones < '0' || ones > '9') {
        return false;
    }
    const int value = ((hundreds - '0') * 100) + ((tens - '0') * 10) + (ones - '0');
    if (value < 1 || value > max_value) {
        return false;
    }
    *out_index = value - 1;
    return true;
}

inline bool lookup_game_texture_spec(std::string_view file_name,
                                     GameTextureSpec* out_spec) {
    if (out_spec == nullptr) {
        return false;
    }

    static constexpr std::array<GameTextureSpec, 9> kArrowSeries = {{
        {88, 15, 128, 16},
        {96, 15, 128, 16},
        {93, 19, 128, 32},
        {100, 24, 128, 32},
        {99, 17, 128, 32},
        {86, 17, 128, 32},
        {85, 14, 128, 16},
        {95, 12, 128, 16},
        {98, 20, 128, 32},
    }};
    static constexpr std::array<GameTextureSpec, 9> kBowSeries = {{
        {88, 144, 128, 256},
        {89, 144, 128, 256},
        {101, 154, 128, 256},
        {123, 143, 128, 256},
        {95, 161, 128, 256},
        {112, 143, 128, 256},
        {91, 139, 128, 256},
        {117, 138, 128, 256},
        {112, 148, 128, 256},
    }};
    static constexpr std::array<GameTextureSpec, 10> kBallLv3Series = {{
        {40, 35, 64, 64},
        {40, 35, 64, 64},
        {40, 35, 64, 64},
        {40, 35, 64, 64},
        {40, 35, 64, 64},
        {40, 35, 64, 64},
        {40, 35, 64, 64},
        {40, 35, 64, 64},
        {40, 35, 64, 64},
        {40, 35, 64, 64},
    }};
    static constexpr std::array<std::pair<std::string_view, GameTextureSpec>, 33> kSpecs = {{
        {"bg_lv0.jpg", {800, 480, 1024, 512}},
        {"bg_lv1.jpg", {145, 480, 256, 512}},
        {"bg_lv2.jpg", {145, 480, 256, 512}},
        {"bg_lv3.jpg", {145, 480, 256, 512}},
        {"river_lv1.jpg", {102, 480, 128, 512}},
        {"river_lv1a.jpg", {102, 480, 128, 512}},
        {"river_lv2.jpg", {202, 480, 256, 512}},
        {"river_lv2a.jpg", {202, 480, 256, 512}},
        {"river_lv3.jpg", {282, 480, 512, 512}},
        {"river_lv3a.jpg", {282, 480, 512, 512}},
        {"blood_panel.png", {260, 70, 512, 128}},
        {"blood_panel_bg.png", {4, 29, 4, 32}},
        {"z_blood_panel_bg.png", {1, 14, 1, 16}},
        {"blood_panel_blue.png", {4, 10, 4, 16}},
        {"blood_panel_red.png", {4, 14, 4, 16}},
        {"button_addmana.png", {64, 64, 64, 64}},
        {"magic_button_fire.png", {80, 81, 128, 128}},
        {"magic_button_fire2.png", {80, 81, 128, 128}},
        {"magic_button_fire3.png", {80, 81, 128, 128}},
        {"magic_button_ice.png", {80, 81, 128, 128}},
        {"magic_button_ice2.png", {80, 81, 128, 128}},
        {"magic_button_ice3.png", {80, 81, 128, 128}},
        {"magic_button_elect.png", {80, 81, 128, 128}},
        {"magic_button_elect2.png", {80, 81, 128, 128}},
        {"magic_button_elect3.png", {80, 81, 128, 128}},
        {"magic_button_lock.png", {80, 81, 128, 128}},
        {"magic_button_lowmana_bg.png", {80, 81, 128, 128}},
        {"magic_button_lowmana_word.png", {80, 81, 128, 128}},
        {"magic_button_flash.png", {94, 88, 128, 128}},
        {"magic_cd.png", {78, 77, 128, 128}},
        {"coin.png", {26, 27, 32, 32}},
        {"mana_stone.png", {27, 30, 32, 32}},
        {"z_number_list.png", {250, 33, 256, 64}},
    }};
    static constexpr std::array<std::pair<std::string_view, GameTextureSpec>, 20> kMoreSpecs = {{
        {"sword_logo.png", {261, 37, 512, 64}},
        {"monster_logo.png", {37, 55, 64, 64}},
        {"wall_broken_1.png", {146, 480, 256, 512}},
        {"wall_broken_2.png", {146, 480, 256, 512}},
        {"gameover_word.png", {451, 115, 512, 128}},
        {"gameover_tips.png", {410, 47, 512, 64}},
        {"warning.png", {476, 76, 512, 128}},
        {"bow_normal.png", {90, 135, 128, 256}},
        {"bow_final.png", {106, 131, 128, 256}},
        {"normal_lv1.png", {31, 29, 32, 32}},
        {"normal_lv2.png", {46, 55, 64, 64}},
        {"normal_lv3.png", {46, 46, 64, 64}},
        {"ring_normal_lv3.png", {91, 67, 128, 128}},
        {"research_bg.jpg", {800, 480, 1024, 512}},
        {"equip_cover_bg.png", {800, 480, 1024, 512}},
        {"equip_cover_bg_fixed.png", {800, 480, 800, 480}},
        {"research_line.png", {1000, 480, 1024, 512}},
        {"research_defender_line.png", {599, 192, 1024, 256}},
        {"research_magic_line.png", {600, 194, 1024, 256}},
        {"research_wall_line.png", {331, 228, 512, 256}},
    }};

    if (file_name == "arrow_normal.png") {
        *out_spec = {85, 9, 128, 16};
        return true;
    }
    if (file_name == "arrow_final.png") {
        *out_spec = {86, 14, 128, 16};
        return true;
    }

    int series_index = -1;
    if (parse_numbered_png(file_name, "arrow_pow_", &series_index) ||
        parse_numbered_png(file_name, "arrow_agi_", &series_index) ||
        parse_numbered_png(file_name, "arrow_mul_", &series_index)) {
        *out_spec = kArrowSeries[static_cast<size_t>(series_index)];
        return true;
    }
    if (parse_numbered_png(file_name, "bow_pow_", &series_index) ||
        parse_numbered_png(file_name, "bow_agi_", &series_index) ||
        parse_numbered_png(file_name, "bow_mul_", &series_index)) {
        *out_spec = kBowSeries[static_cast<size_t>(series_index)];
        return true;
    }
    if (parse_numbered_png_3(file_name, "ball_lv3_", &series_index, 10)) {
        *out_spec = kBallLv3Series[static_cast<size_t>(series_index)];
        return true;
    }

    for (const auto& entry : kSpecs) {
        if (entry.first == file_name) {
            *out_spec = entry.second;
            return true;
        }
    }
    for (const auto& entry : kMoreSpecs) {
        if (entry.first == file_name) {
            *out_spec = entry.second;
            return true;
        }
    }
    return false;
}

inline int game_texture_width_px(const TextureHandle& tex,
                                 std::string_view file_name) {
    GameTextureSpec spec{};
    if (lookup_game_texture_spec(file_name, &spec)) {
        return spec.width;
    }
    return tex.width;
}

inline int game_texture_height_px(const TextureHandle& tex,
                                  std::string_view file_name) {
    GameTextureSpec spec{};
    if (lookup_game_texture_spec(file_name, &spec)) {
        return spec.height;
    }
    return tex.height;
}

inline float game_texture_u1(const TextureHandle& tex,
                             std::string_view file_name) {
    (void)tex;
    GameTextureSpec spec{};
    if (lookup_game_texture_spec(file_name, &spec) && spec.wrap_width > 0) {
        return static_cast<float>(spec.width) /
               static_cast<float>(spec.wrap_width);
    }
    return 1.0f;
}

inline float game_texture_v0(const TextureHandle& tex,
                             std::string_view file_name) {
    (void)tex;
    GameTextureSpec spec{};
    if (lookup_game_texture_spec(file_name, &spec) && spec.wrap_height > 0) {
        return static_cast<float>(spec.height) /
               static_cast<float>(spec.wrap_height);
    }
    return 1.0f;
}

inline bool draw_game_texture_quad(const TextureHandle& tex,
                                   std::string_view file_name,
                                   float x,
                                   float y,
                                   float w,
                                   float h,
                                   float alpha = 1.0f) {
    if (!tex.valid || tex.id == 0) {
        return false;
    }
    GameTextureSpec spec{};
    if (lookup_game_texture_spec(file_name, &spec)) {
        draw_textured_quad_uv(
            tex.id,
            x,
            y,
            w,
            h,
            0.0f,
            static_cast<float>(spec.height) /
                static_cast<float>(spec.wrap_height),
            static_cast<float>(spec.width) /
                static_cast<float>(spec.wrap_width),
            0.0f,
            alpha
        );
    } else {
        draw_textured_quad(tex.id, x, y, w, h, alpha);
    }
    return true;
}

inline bool draw_game_texture_quad_tinted(const TextureHandle& tex,
                                          std::string_view file_name,
                                          float x,
                                          float y,
                                          float w,
                                          float h,
                                          float r,
                                          float g,
                                          float b,
                                          float alpha = 1.0f) {
    if (!tex.valid || tex.id == 0) {
        return false;
    }
    GameTextureSpec spec{};
    if (lookup_game_texture_spec(file_name, &spec)) {
        const GLfloat vertices[] = {
            x, y, 0.0f,
            x + w, y, 0.0f,
            x, y + h, 0.0f,
            x + w, y + h, 0.0f
        };
        const GLfloat uvs[] = {
            0.0f,
            static_cast<float>(spec.height) / static_cast<float>(spec.wrap_height),
            static_cast<float>(spec.width) / static_cast<float>(spec.wrap_width),
            static_cast<float>(spec.height) / static_cast<float>(spec.wrap_height),
            0.0f,
            0.0f,
            static_cast<float>(spec.width) / static_cast<float>(spec.wrap_width),
            0.0f
        };
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glColor4f(r, g, b, alpha);
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        glTexCoordPointer(2, GL_FLOAT, 0, uvs);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    } else {
        draw_textured_quad_tinted(tex.id, x, y, w, h, r, g, b, alpha);
    }
    return true;
}

inline bool draw_game_texture_quad_pivoted(const TextureHandle& tex,
                                           std::string_view file_name,
                                           float pivot_x,
                                           float pivot_y,
                                           float w,
                                           float h,
                                           float pivot_offset_x,
                                           float pivot_offset_y,
                                           float angle_deg,
                                           float alpha = 1.0f) {
    if (!tex.valid || tex.id == 0) {
        return false;
    }
    GameTextureSpec spec{};
    if (lookup_game_texture_spec(file_name, &spec)) {
        draw_textured_quad_pivoted_uv(
            tex.id,
            pivot_x,
            pivot_y,
            w,
            h,
            0.0f,
            static_cast<float>(spec.height) /
                static_cast<float>(spec.wrap_height),
            static_cast<float>(spec.width) /
                static_cast<float>(spec.wrap_width),
            0.0f,
            pivot_offset_x,
            pivot_offset_y,
            angle_deg,
            alpha
        );
    } else {
        draw_textured_quad_pivoted(
            tex.id,
            pivot_x,
            pivot_y,
            w,
            h,
            pivot_offset_x,
            pivot_offset_y,
            angle_deg,
            alpha
        );
    }
    return true;
}

} // namespace defender
