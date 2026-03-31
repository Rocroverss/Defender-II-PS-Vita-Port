#include "scenes/shop_scene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <utility>

#include "engine/abstract_game.hpp"
#include "engine/font_renderer.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/achv_mng.hpp"
#include "game/audio_manager.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "game/sounds.hpp"

namespace defender {
namespace {

constexpr const char* kShopBg = "assets/imgs_480_800/shop/shop_bg_fixed.jpg";

constexpr std::array<ShopScene::ShopButton, 7> kButtons = {{
    {"assets/imgs_480_800/shop/shop_bt1_up.png", "assets/imgs_480_800/shop/shop_bt1_down.png", 30.0f, 310.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt2_up.png", "assets/imgs_480_800/shop/shop_bt2_down.png", 30.0f, 190.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt3_up.png", "assets/imgs_480_800/shop/shop_bt3_down.png", 30.0f, 70.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt4_up.png", "assets/imgs_480_800/shop/shop_bt4_down.png", 410.0f, 310.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt5_up.png", "assets/imgs_480_800/shop/shop_bt5_down.png", 410.0f, 190.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt6_up.png", "assets/imgs_480_800/shop/shop_bt6_down.png", 410.0f, 70.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_btfree_up.png", "assets/imgs_480_800/shop/shop_btfree_down.png", 600.0f, 10.0f, 170.0f, 54.0f}
}};

constexpr std::array<const char*, 6> kPriceText = {
    "$1.99", "$4.99", "$9.99", "$1.99", "$4.99", "$14.99"
};

constexpr std::array<int, 3> kCoinPack = {10000, 25000, 60000};
constexpr std::array<int, 3> kStonePack = {25, 75, 200};

void draw_background_cover(const char* path) {
    const auto& tex = TextureCache::instance().get(path);
    if (tex.valid) {
        draw_textured_quad(tex.id, 0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 1.0f);
    } else {
        draw_quad(0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 0.08f, 0.08f, 0.08f, 1.0f);
    }
}

void draw_sprite(const char* path, float x, float y, float w, float h, float alpha = 1.0f) {
    const auto& tex = TextureCache::instance().get(path);
    const float dx = Scene::get_x(x);
    const float dy = Scene::get_y(y);
    const float dw = Scene::get_x(w);
    const float dh = Scene::get_y(h);
    if (tex.valid) {
        draw_textured_quad(tex.id, dx, dy, dw, dh, alpha);
    } else {
        draw_quad(dx, dy, dw, dh, 0.2f, 0.2f, 0.2f, alpha);
    }
}

} // namespace

ShopScene::ShopScene(TransitionRequest transition_cb) : transition_cb_(std::move(transition_cb)) {}

bool ShopScene::hit_test(const ShopButton& button, float px, float py) {
    const float x0 = Scene::get_x(button.x);
    const float y0 = Scene::get_y(button.y);
    const float x1 = Scene::get_x(button.x + button.w);
    const float y1 = Scene::get_y(button.y + button.h);
    return px >= x0 && px <= x1 && py >= y0 && py <= y1;
}

void ShopScene::draw_button(const ShopButton& button, bool pressed, float alpha) {
    draw_sprite(pressed ? button.down : button.up, button.x, button.y, button.w, button.h, alpha);
}

void ShopScene::purchase(int id) {
    if (id < 0 || id >= 7) {
        return;
    }

    if (id < 3) {
        const int add = kCoinPack[static_cast<size_t>(id)];
        Param::gold += add;
        Param::cost_coin += add;
        Save::save_data(Save::GOLD, Param::gold);
        Save::save_data(Save::COST_COIN, Param::cost_coin);
        AchvMng::check_achv_in_game(0);
        return;
    }

    if (id < 6) {
        const int add = kStonePack[static_cast<size_t>(id - 3)];
        Param::stone += add;
        Param::cost_stone += add;
        Save::save_data(Save::STONE, Param::stone);
        Save::save_data(Save::COST_STONE, Param::cost_stone);
        AchvMng::check_achv_in_game(1);
        return;
    }

    Param::stone += 5;
    Save::save_data(Save::STONE, Param::stone);
    AchvMng::check_achv_in_game(1);
}

bool ShopScene::touch(const TouchEvent& event) {
    if (event.action == TouchAction::Down) {
        pressed_id_ = -1;
        for (int i = 0; i < static_cast<int>(kButtons.size()); ++i) {
            if (hit_test(kButtons[static_cast<size_t>(i)], event.x1, event.y1)) {
                pressed_id_ = i;
                return true;
            }
        }
        return true;
    }

    if (event.action == TouchAction::Up) {
        const int released_id = pressed_id_;
        pressed_id_ = -1;
        if (released_id >= 0 && hit_test(kButtons[static_cast<size_t>(released_id)], event.x1, event.y1)) {
            AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
            purchase(released_id);
            return true;
        }
    }

    return true;
}

void ShopScene::draw() {
    draw_background_cover(kShopBg);

    for (int i = 0; i < static_cast<int>(kButtons.size()); ++i) {
        float alpha = 1.0f;
        if (i == 6) {
            const float t = std::abs(500.0f - static_cast<float>(AbstractGame::game_time_ms() % 1000ULL)) / 500.0f;
            alpha = 0.70f + (0.30f * (1.0f - t));
        }
        draw_button(kButtons[static_cast<size_t>(i)], pressed_id_ == i, alpha);
    }

    char line[64];
    auto& fonts = FontRenderer::instance();

    const std::array<std::pair<float, float>, 6> price_pos = {{
        {260.0f, 355.0f},
        {260.0f, 235.0f},
        {260.0f, 115.0f},
        {640.0f, 355.0f},
        {640.0f, 235.0f},
        {640.0f, 115.0f}
    }};
    for (int i = 0; i < 6; ++i) {
        fonts.draw_text(
            FontFaceId::Cooper,
            kPriceText[static_cast<size_t>(i)],
            get_x(price_pos[static_cast<size_t>(i)].first),
            get_y(price_pos[static_cast<size_t>(i)].second),
            get_y(18.0f),
            1.0f,
            1.0f,
            1.0f,
            0.98f
        );
    }

    std::snprintf(line, sizeof(line), "GOLD %d", std::max(0, Param::gold));
    fonts.draw_text(
        FontFaceId::Ants,
        line,
        get_x(28.0f),
        get_y(30.0f),
        get_y(20.0f),
        0.95f,
        0.95f,
        0.95f,
        0.98f
    );

    std::snprintf(line, sizeof(line), "STONE %d", std::max(0, Param::stone));
    fonts.draw_text(
        FontFaceId::Ants,
        line,
        get_x(220.0f),
        get_y(30.0f),
        get_y(20.0f),
        0.95f,
        0.95f,
        0.95f,
        0.98f
    );

    fonts.draw_text(
        FontFaceId::Cooper,
        "SHOP",
        get_x(715.0f),
        get_y(34.0f),
        get_y(22.0f),
        0.97f,
        0.90f,
        0.70f,
        0.98f
    );
}

void ShopScene::update() {
    pulse_ += static_cast<float>(AbstractGame::last_span_ms()) / 600.0f;
    if (pulse_ > 2.0f) {
        pulse_ -= 2.0f;
    }
}

void ShopScene::reset() {
    pressed_id_ = -1;
    pulse_ = 0.0f;
    scene_x = 0.0f;
    scene_y = 0.0f;
    scene_scale = 1.0f;
    scene_alpha = 1.0f;
}

} // namespace defender
