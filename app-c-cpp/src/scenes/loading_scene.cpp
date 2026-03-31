#include "scenes/loading_scene.hpp"

#include <algorithm>
#include <utility>

#include "engine/abstract_game.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/param.hpp"

namespace defender {
namespace {

constexpr const char* kLine = "assets/imgs_480_800/mode/load_line.png";
constexpr const char* kLogoLight = "assets/imgs_480_800/mode/load_logo_light.png";
constexpr const char* kDh = "assets/imgs_480_800/mode/load_droidhen.png";
constexpr const char* kDhLight = "assets/imgs_480_800/mode/load_droidhen_light.png";
constexpr const char* kGame = "assets/imgs_480_800/mode/load_game.png";
constexpr const char* kGameLight = "assets/imgs_480_800/mode/load_game_light.png";

constexpr const char* kLogoFrames[] = {
    "assets/imgs_480_800/mode/load_logo1.png",
    "assets/imgs_480_800/mode/load_logo2.png",
    "assets/imgs_480_800/mode/load_logo3.png",
    "assets/imgs_480_800/mode/load_logo4.png",
    "assets/imgs_480_800/mode/load_logo5.png"
};

constexpr float kLineW = 480.0f;
constexpr float kLineH = 50.0f;
constexpr float kLogoFrameW = 172.0f;
constexpr float kLogoFrameH = 204.0f;
constexpr float kLogoLightW = 250.0f;
constexpr float kLogoLightH = 287.0f;
constexpr float kDhW = 489.0f;
constexpr float kDhH = 41.0f;
constexpr float kDhLightW = 513.0f;
constexpr float kDhLightH = 63.0f;
constexpr float kGameW = 460.0f;
constexpr float kGameH = 24.5f;
constexpr float kGameLightW = 460.0f;
constexpr float kGameLightH = 47.0f;

} // namespace

LoadingScene::LoadingScene(TransitionRequest transition_cb) : transition_cb_(std::move(transition_cb)) {}

void LoadingScene::draw_centered(const char* path, float cx, float cy, float w, float h, float alpha) {
    const auto& tex = TextureCache::instance().get(path);
    const float dw = Scene::get_x(w);
    const float dh = Scene::get_y(h);
    const float dx = Scene::get_x(cx) - (dw * 0.5f);
    const float dy = Scene::get_y(cy) - (dh * 0.5f);
    if (tex.valid) {
        draw_textured_quad(tex.id, dx, dy, dw, dh, alpha);
    } else {
        draw_quad(dx, dy, dw, dh, 0.15f, 0.15f, 0.15f, alpha);
    }
}

bool LoadingScene::touch(const TouchEvent&) {
    return false;
}

void LoadingScene::draw() {
    draw_quad(0.0f, 0.0f, screen_width, screen_height, 0.0f, 0.0f, 0.0f, 1.0f);

    if (comic_time_ms_ <= 0) {
        return;
    }

    const float a = std::clamp(all_alpha_, 0.0f, 1.0f);
    const float line_y = 288.0f;
    const float logo_x = 400.0f;
    const float logo_y = line_y + 10.0f;
    const float dh_y = 150.0f;
    const float game_y = 96.0f;

    if (line_alpha_ > 0.0f) {
        draw_centered(kLine, line_x_, line_y, kLineW, kLineH, std::clamp(line_alpha_ * a, 0.0f, 1.0f));
    }

    if (logo_alpha_ > 0.0f) {
        int frame = std::clamp((comic_time_ms_ - 300) / 50, 0, 4);
        draw_centered(kLogoFrames[frame], logo_x, logo_y, kLogoFrameW, kLogoFrameH, std::clamp(logo_alpha_ * a, 0.0f, 1.0f));
    }

    if (dh_alpha_ > 0.0f) {
        draw_centered(kDh, 400.0f, dh_y, kDhW, kDhH, std::clamp(dh_alpha_ * a, 0.0f, 1.0f));
    }
    if (game_alpha_ > 0.0f) {
        draw_centered(kGame, 400.0f, game_y, kGameW, kGameH, std::clamp(game_alpha_ * a, 0.0f, 1.0f));
    }
    if (light_alpha_ > 0.0f) {
        const float la = std::clamp(light_alpha_ * a, 0.0f, 1.0f);
        draw_centered(kLogoLight, logo_x, logo_y, kLogoLightW, kLogoLightH, la);
        draw_centered(kDhLight, 400.0f, dh_y, kDhLightW, kDhLightH, la);
        draw_centered(kGameLight, 400.0f, game_y, kGameLightW, kGameLightH, la);
    }
}

void LoadingScene::update() {
    const int dt = static_cast<int>(AbstractGame::last_span_ms());
    comic_time_ms_ += dt;

    line_x_ += (800.0f * static_cast<float>(dt)) / 300.0f;
    if (line_x_ > 400.0f) {
        line_x_ = 400.0f;
        line_alpha_ -= static_cast<float>(dt) / 300.0f;
        logo_alpha_ = 1.0f;
    }
    if (comic_time_ms_ > 1000) {
        dh_alpha_ += static_cast<float>(dt) / 500.0f;
    }
    if (comic_time_ms_ > 1500) {
        game_alpha_ += static_cast<float>(dt) / 500.0f;
    }
    if (comic_time_ms_ > 2000 && comic_time_ms_ < 2500) {
        light_alpha_ += static_cast<float>(dt) / 500.0f;
    }
    if (comic_time_ms_ > 2500) {
        light_alpha_ -= static_cast<float>(dt) / 500.0f;
    }
    if (comic_time_ms_ > 4000) {
        all_alpha_ -= static_cast<float>(dt) / 500.0f;
    }

    line_alpha_ = std::clamp(line_alpha_, 0.0f, 1.0f);
    logo_alpha_ = std::clamp(logo_alpha_, 0.0f, 1.0f);
    dh_alpha_ = std::clamp(dh_alpha_, 0.0f, 1.0f);
    game_alpha_ = std::clamp(game_alpha_, 0.0f, 1.0f);
    light_alpha_ = std::clamp(light_alpha_, 0.0f, 1.0f);

    if (all_alpha_ < 0.0f && !transitioned_) {
        transitioned_ = true;
        if (transition_cb_) {
            transition_cb_(Param::SCENE_COVER, 0);
        }
    }
}

void LoadingScene::reset() {
    comic_time_ms_ = 0;
    line_x_ = -400.0f;
    line_alpha_ = 1.0f;
    logo_alpha_ = 0.0f;
    dh_alpha_ = 0.0f;
    game_alpha_ = 0.0f;
    light_alpha_ = 0.0f;
    all_alpha_ = 1.0f;
    transitioned_ = false;
    scene_x = 0.0f;
    scene_y = 0.0f;
    scene_scale = 1.0f;
    scene_alpha = 1.0f;
}

} // namespace defender
