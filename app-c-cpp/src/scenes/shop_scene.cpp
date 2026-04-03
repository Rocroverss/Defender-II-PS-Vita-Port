#include "scenes/shop_scene.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <string>
#include <utility>

#include "engine/abstract_game.hpp"
#include "engine/font_renderer.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/audio_manager.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "game/sounds.hpp"

namespace defender {
namespace {

constexpr const char* kShopBg = "assets/imgs_480_800/shop/shop_bg_fixed.jpg";

constexpr std::array<ShopScene::ShopButton, 6> kButtons = {{
    {"assets/imgs_480_800/shop/shop_bt1_up.png", "assets/imgs_480_800/shop/shop_bt1_down.png", 30.0f, 310.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt2_up.png", "assets/imgs_480_800/shop/shop_bt2_down.png", 30.0f, 190.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt3_up.png", "assets/imgs_480_800/shop/shop_bt3_down.png", 30.0f, 70.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt4_up.png", "assets/imgs_480_800/shop/shop_bt4_down.png", 410.0f, 310.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt5_up.png", "assets/imgs_480_800/shop/shop_bt5_down.png", 410.0f, 190.0f, 350.0f, 110.0f},
    {"assets/imgs_480_800/shop/shop_bt6_up.png", "assets/imgs_480_800/shop/shop_bt6_down.png", 410.0f, 70.0f, 350.0f, 110.0f}
}};

constexpr std::array<int, 3> kCoinPack = {10000, 25000, 60000};
constexpr std::array<int, 3> kStonePack = {25, 75, 200};

constexpr float kOfferButtonX = 225.0f;
constexpr float kGoldOfferY = 120.0f;
constexpr float kStoneOfferY = 250.0f;
constexpr float kInfoBoxX = 175.0f;
constexpr float kInfoBoxY = 58.0f;
constexpr float kInfoBoxW = 450.0f;
constexpr float kInfoBoxH = 26.0f;
constexpr float kClaimedBoxX = 170.0f;
constexpr float kClaimedBoxY = 392.0f;
constexpr float kClaimedBoxW = 460.0f;
constexpr float kClaimedBoxH = 28.0f;

constexpr float kConfirmPanelX = 165.0f;
constexpr float kConfirmPanelY = 120.0f;
constexpr float kConfirmPanelW = 470.0f;
constexpr float kConfirmPanelH = 210.0f;
constexpr float kConfirmButtonY = 260.0f;
constexpr float kConfirmButtonW = 132.0f;
constexpr float kConfirmButtonH = 46.0f;
constexpr float kConfirmButtonGap = 18.0f;
constexpr float kConfirmYesX = kConfirmPanelX + ((kConfirmPanelW - ((kConfirmButtonW * 2.0f) + kConfirmButtonGap)) * 0.5f);
constexpr float kConfirmYesY = kConfirmButtonY;
constexpr float kConfirmNoX = kConfirmYesX + kConfirmButtonW + kConfirmButtonGap;
constexpr float kConfirmNoY = kConfirmButtonY;

constexpr int kClaimTypeNone = 0;
constexpr int kClaimTypeGold = 1;
constexpr int kClaimTypeStone = 2;

struct ConfirmButtonLayout {
    float x;
    float y;
    float w;
    float h;
};

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

bool hit_box(float x, float y, float w, float h, float px, float py) {
    const float x0 = Scene::get_x(x);
    const float y0 = Scene::get_y(y);
    const float x1 = Scene::get_x(x + w);
    const float y1 = Scene::get_y(y + h);
    return px >= x0 && px <= x1 && py >= y0 && py <= y1;
}

void draw_centered_text_box(
    FontRenderer& fonts,
    FontFaceId font,
    const std::string& text,
    float box_x,
    float box_y,
    float box_w,
    float box_h,
    float font_size,
    float r,
    float g,
    float b,
    float a
) {
    const float pixel_height = Scene::get_y(font_size);
    const float text_width = fonts.measure_text_width(font, text, pixel_height);
    const float draw_x = Scene::get_x(box_x) + std::max(0.0f, (Scene::get_x(box_w) - text_width) * 0.5f);
    const float draw_y = Scene::get_y(box_y + ((box_h - font_size) * 0.5f) + font_size);
    fonts.draw_text(font, text, draw_x + Scene::get_x(1.0f), draw_y + Scene::get_y(1.0f), pixel_height, 0.0f, 0.0f, 0.0f, a);
    fonts.draw_text(font, text, draw_x, draw_y, pixel_height, r, g, b, a);
}

void draw_confirm_button(FontRenderer& fonts,
                         float x,
                         float y,
                         float w,
                         float h,
                         bool pressed,
                         const std::string& text,
                         float outer_r,
                         float outer_g,
                         float outer_b,
                         float inner_r,
                         float inner_g,
                         float inner_b) {
    const float dx = Scene::get_x(x);
    const float dy = Scene::get_y(y);
    const float dw = Scene::get_x(w);
    const float dh = Scene::get_y(h);
    const float inset = Scene::get_xy(2.0f);
    const float gloss_h = Scene::get_y(h * 0.42f);

    draw_quad(dx + Scene::get_x(2.0f), dy - Scene::get_y(2.0f), dw, dh, 0.0f, 0.0f, 0.0f, 0.28f);
    draw_quad(dx, dy, dw, dh, outer_r, outer_g, outer_b, 0.98f);
    draw_quad(
        dx + inset,
        dy + inset,
        std::max(0.0f, dw - (inset * 2.0f)),
        std::max(0.0f, dh - (inset * 2.0f)),
        pressed ? inner_r * 0.78f : inner_r,
        pressed ? inner_g * 0.78f : inner_g,
        pressed ? inner_b * 0.78f : inner_b,
        0.99f
    );
    draw_quad(
        dx + inset,
        dy + inset,
        std::max(0.0f, dw - (inset * 2.0f)),
        std::max(0.0f, gloss_h - inset),
        1.0f,
        1.0f,
        1.0f,
        pressed ? 0.08f : 0.16f
    );

    draw_centered_text_box(
        fonts,
        FontFaceId::Cooper,
        text,
        x,
        y + 3.0f,
        w,
        h - 6.0f,
        18.0f,
        0.99f,
        0.96f,
        0.86f,
        0.98f
    );
}

const ShopConfirmLayoutTweak& confirm_layout_tweak(ShopConfirmLayoutElement element) {
    return kShopConfirmLayoutTweaks[static_cast<int>(element)];
}

ConfirmButtonLayout confirm_button_layout(ShopConfirmLayoutElement element) {
    const ShopConfirmLayoutTweak& tweak = confirm_layout_tweak(element);
    const bool claim = element == ShopConfirmLayoutElement::ClaimButton;
    return {
        (claim ? kConfirmYesX : kConfirmNoX) + tweak.x,
        (claim ? kConfirmYesY : kConfirmNoY) + tweak.y,
        kConfirmButtonW * tweak.scale,
        kConfirmButtonH * tweak.scale
    };
}

int64_t current_utc_day_key() {
    using namespace std::chrono;
    const auto now = system_clock::now().time_since_epoch();
    return duration_cast<hours>(now).count() / 24;
}

uint64_t mix_seed(uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31U);
}

int pick_offer_index(int64_t day_key, uint64_t salt, int offset) {
    const uint64_t mixed = mix_seed(static_cast<uint64_t>(day_key) ^ salt);
    return offset + static_cast<int>(mixed % 3ULL);
}

std::string format_amount(int value) {
    std::string digits = std::to_string(std::max(0, value));
    std::string out;
    out.reserve(digits.size() + (digits.size() / 3U));
    int group_count = 0;
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        if (group_count == 3) {
            out.push_back(',');
            group_count = 0;
        }
        out.push_back(*it);
        ++group_count;
    }
    std::reverse(out.begin(), out.end());
    return out;
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

void ShopScene::refresh_daily_offer() {
    const int64_t day_key = current_utc_day_key();
    if (offer_day_key_ != day_key) {
        offer_day_key_ = day_key;
        gold_offer_id_ = pick_offer_index(day_key, 0x4f1bbcdcb1234aa1ULL, 0);
        stone_offer_id_ = pick_offer_index(day_key, 0x8e88ad6ca8d3f527ULL, 3);
    }

    claimed_day_key_ = static_cast<int64_t>(Save::load_data(Save::DAILY_SHOP_CLAIM_DAY));
    claimed_reward_type_ = Save::load_data(Save::DAILY_SHOP_CLAIM_TYPE);
    if (claimed_day_key_ != offer_day_key_) {
        claimed_reward_type_ = kClaimTypeNone;
    }
}

void ShopScene::show_confirmation(int id) {
    if (!is_offer_button(id) || is_claimed_today()) {
        return;
    }
    confirm_choice_id_ = id;
    confirm_pressed_id_ = -1;
    confirm_visible_ = true;
}

void ShopScene::claim_reward(int id) {
    if (!is_offer_button(id) || is_claimed_today()) {
        return;
    }

    if (is_gold_offer(id)) {
        Param::gold += kCoinPack[static_cast<size_t>(id)];
        Save::save_data(Save::GOLD, Param::gold);
        claimed_reward_type_ = kClaimTypeGold;
    } else {
        Param::stone += kStonePack[static_cast<size_t>(id - 3)];
        Save::save_data(Save::STONE, Param::stone);
        claimed_reward_type_ = kClaimTypeStone;
    }

    claimed_day_key_ = offer_day_key_;
    Save::save_data(Save::DAILY_SHOP_CLAIM_DAY, static_cast<int>(offer_day_key_));
    Save::save_data(Save::DAILY_SHOP_CLAIM_TYPE, claimed_reward_type_);
}

ShopScene::ShopButton ShopScene::display_button_for_offer(int id) const {
    if (id < 0 || id >= static_cast<int>(kButtons.size())) {
        return {};
    }

    ShopButton button = kButtons[static_cast<size_t>(id)];
    button.x = kOfferButtonX;
    button.y = is_gold_offer(id) ? kGoldOfferY : kStoneOfferY;
    return button;
}

bool ShopScene::is_offer_button(int id) const {
    return id == gold_offer_id_ || id == stone_offer_id_;
}

bool ShopScene::is_claimed_today() const {
    return claimed_day_key_ == offer_day_key_ && claimed_reward_type_ != kClaimTypeNone;
}

bool ShopScene::is_claimed_choice(int id) const {
    if (!is_claimed_today()) {
        return false;
    }
    if (claimed_reward_type_ == kClaimTypeGold) {
        return is_gold_offer(id);
    }
    if (claimed_reward_type_ == kClaimTypeStone) {
        return !is_gold_offer(id);
    }
    return false;
}

bool ShopScene::is_gold_offer(int id) const {
    return id >= 0 && id < 3;
}

std::string ShopScene::reward_text(int id) const {
    if (id >= 0 && id < 3) {
        return format_amount(kCoinPack[static_cast<size_t>(id)]) + " GOLD";
    }
    if (id >= 3 && id < 6) {
        return format_amount(kStonePack[static_cast<size_t>(id - 3)]) + " CRYSTALS";
    }
    return "DAILY";
}

bool ShopScene::touch_confirmation(const TouchEvent& event) {
    if (!confirm_visible_) {
        return false;
    }

    const ConfirmButtonLayout claim_button = confirm_button_layout(ShopConfirmLayoutElement::ClaimButton);
    const ConfirmButtonLayout cancel_button = confirm_button_layout(ShopConfirmLayoutElement::CancelButton);

    if (event.action == TouchAction::Down) {
        confirm_pressed_id_ = -1;
        if (hit_box(claim_button.x, claim_button.y, claim_button.w, claim_button.h, event.x1, event.y1)) {
            confirm_pressed_id_ = 0;
        } else if (hit_box(cancel_button.x, cancel_button.y, cancel_button.w, cancel_button.h, event.x1, event.y1)) {
            confirm_pressed_id_ = 1;
        }
        return true;
    }

    if (event.action == TouchAction::Up) {
        const int pressed = confirm_pressed_id_;
        confirm_pressed_id_ = -1;

        if (pressed == 0 && hit_box(claim_button.x, claim_button.y, claim_button.w, claim_button.h, event.x1, event.y1)) {
            AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
            claim_reward(confirm_choice_id_);
            confirm_visible_ = false;
            confirm_choice_id_ = -1;
            return true;
        }

        if (pressed == 1 && hit_box(cancel_button.x, cancel_button.y, cancel_button.w, cancel_button.h, event.x1, event.y1)) {
            AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
            confirm_visible_ = false;
            confirm_choice_id_ = -1;
            return true;
        }
    }

    return true;
}

bool ShopScene::touch(const TouchEvent& event) {
    refresh_daily_offer();

    if (confirm_visible_) {
        return touch_confirmation(event);
    }

    if (event.action == TouchAction::Down) {
        pressed_id_ = -1;
        if (!is_claimed_today()) {
            for (const int offer_id : {gold_offer_id_, stone_offer_id_}) {
                const ShopButton button = display_button_for_offer(offer_id);
                if (hit_test(button, event.x1, event.y1)) {
                    pressed_id_ = offer_id;
                    break;
                }
            }
        }
        return true;
    }

    if (event.action == TouchAction::Up) {
        const int released_id = pressed_id_;
        pressed_id_ = -1;
        if (released_id >= 0 && hit_test(display_button_for_offer(released_id), event.x1, event.y1)) {
            AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
            show_confirmation(released_id);
            return true;
        }
    }

    return true;
}

void ShopScene::draw() {
    refresh_daily_offer();
    draw_background_cover(kShopBg);

    const float highlight = 0.82f + (0.18f * (1.0f - std::abs(500.0f - static_cast<float>(AbstractGame::game_time_ms() % 1000ULL)) / 500.0f));

    auto& fonts = FontRenderer::instance();

    draw_centered_text_box(
        fonts,
        FontFaceId::Ants,
        "Choose one reward for today.",
        kInfoBoxX,
        kInfoBoxY,
        kInfoBoxW,
        kInfoBoxH,
        17.0f,
        0.97f,
        0.94f,
        0.86f,
        0.96f
    );

    for (const int offer_id : {gold_offer_id_, stone_offer_id_}) {
        const ShopButton button = display_button_for_offer(offer_id);
        float alpha = highlight;
        if (is_claimed_today() && !is_claimed_choice(offer_id)) {
            alpha = 0.45f;
        } else if (is_claimed_choice(offer_id)) {
            alpha = 0.88f;
        }

        draw_button(button, pressed_id_ == offer_id, alpha);
    }

    char line[64];
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

    if (is_claimed_today()) {
        draw_centered_text_box(
            fonts,
            FontFaceId::Ants,
            "Today's reward has already been chosen.",
            kClaimedBoxX,
            kClaimedBoxY,
            kClaimedBoxW,
            kClaimedBoxH,
            16.0f,
            0.97f,
            0.94f,
            0.86f,
            0.96f
        );
    }

    if (!confirm_visible_) {
        return;
    }

    draw_quad(0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 0.0f, 0.0f, 0.0f, 0.62f);
    draw_quad(Scene::get_x(kConfirmPanelX), Scene::get_y(kConfirmPanelY), Scene::get_x(kConfirmPanelW), Scene::get_y(kConfirmPanelH), 0.12f, 0.10f, 0.08f, 0.94f);
    draw_quad(Scene::get_x(kConfirmPanelX + 4.0f), Scene::get_y(kConfirmPanelY + 4.0f), Scene::get_x(kConfirmPanelW - 8.0f), Scene::get_y(kConfirmPanelH - 8.0f), 0.25f, 0.20f, 0.14f, 0.96f);

    const std::string confirm_reward = reward_text(confirm_choice_id_);
    const std::string confirm_title = is_gold_offer(confirm_choice_id_)
        ? "Claim today's free gold reward?"
        : "Claim today's free crystal reward?";

    draw_centered_text_box(
        fonts,
        FontFaceId::Cooper,
        confirm_title,
        kConfirmPanelX + 20.0f,
        kConfirmPanelY + 56.0f,
        kConfirmPanelW - 40.0f,
        28.0f,
        20.0f,
        0.98f,
        0.94f,
        0.82f,
        0.98f
    );
    draw_centered_text_box(
        fonts,
        FontFaceId::Cooper,
        confirm_reward,
        kConfirmPanelX + 40.0f,
        kConfirmPanelY + 108.0f,
        kConfirmPanelW - 80.0f,
        28.0f,
        22.0f,
        1.0f,
        0.97f,
        0.72f,
        0.98f
    );
    draw_centered_text_box(
        fonts,
        FontFaceId::Ants,
        "You can only take one free shop reward each day.",
        kConfirmPanelX + 24.0f,
        kConfirmPanelY + 150.0f,
        kConfirmPanelW - 48.0f,
        22.0f,
        14.0f,
        0.93f,
        0.93f,
        0.93f,
        0.96f
    );

    const bool yes_pressed = confirm_pressed_id_ == 0;
    const bool no_pressed = confirm_pressed_id_ == 1;
    const ConfirmButtonLayout claim_button = confirm_button_layout(ShopConfirmLayoutElement::ClaimButton);
    const ConfirmButtonLayout cancel_button = confirm_button_layout(ShopConfirmLayoutElement::CancelButton);
    draw_confirm_button(
        fonts,
        claim_button.x,
        claim_button.y,
        claim_button.w,
        claim_button.h,
        yes_pressed,
        "CLAIM",
        0.20f,
        0.13f,
        0.05f,
        0.74f,
        0.50f,
        0.15f
    );
    draw_confirm_button(
        fonts,
        cancel_button.x,
        cancel_button.y,
        cancel_button.w,
        cancel_button.h,
        no_pressed,
        "CANCEL",
        0.11f,
        0.12f,
        0.15f,
        0.33f,
        0.36f,
        0.42f
    );
}

void ShopScene::update() {
    refresh_daily_offer();
    pulse_ += static_cast<float>(AbstractGame::last_span_ms()) / 600.0f;
    if (pulse_ > 2.0f) {
        pulse_ -= 2.0f;
    }
}

void ShopScene::reset() {
    Param::gold = Save::load_data(Save::GOLD);
    Param::stone = Save::load_data(Save::STONE);

    refresh_daily_offer();
    pressed_id_ = -1;
    confirm_choice_id_ = -1;
    confirm_pressed_id_ = -1;
    confirm_visible_ = false;
    pulse_ = 0.0f;
    scene_x = 0.0f;
    scene_y = 0.0f;
    scene_scale = 1.0f;
    scene_alpha = 1.0f;
}

} // namespace defender
