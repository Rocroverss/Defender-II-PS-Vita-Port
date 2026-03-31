#include "scenes/research_scene.hpp"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <string>
#include <utility>

#include "engine/abstract_game.hpp"
#include "engine/font_renderer.hpp"
#include "engine/render_utils.hpp"
#include "game/audio_manager.hpp"
#include "game/help_overlay.hpp"
#include "game/item_param.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "game/sounds.hpp"
#include "scenes/research/research_render.hpp"

namespace defender {
namespace {

constexpr const char* kResearchBg = "assets/imgs_480_800/research/research_bg_fixed.jpg";
constexpr const char* kContinueUp = "assets/imgs_480_800/research/research_button_continue_up.png";
constexpr const char* kContinueDown = "assets/imgs_480_800/research/research_button_continue_down.png";
constexpr const char* kContinueUpKr = "assets/imgs_480_800/research/research_button_continue_up_kr.png";
constexpr const char* kContinueDownKr = "assets/imgs_480_800/research/research_button_continue_down_kr.png";
constexpr const char* kDefenderUp = "assets/imgs_480_800/research/research_button_defender_up.png";
constexpr const char* kDefenderDown = "assets/imgs_480_800/research/research_button_defender_down.png";
constexpr const char* kMagicUp = "assets/imgs_480_800/research/research_button_magic_up.png";
constexpr const char* kMagicDown = "assets/imgs_480_800/research/research_button_magic_down.png";
constexpr const char* kWallUp = "assets/imgs_480_800/research/research_button_wall_up.png";
constexpr const char* kWallDown = "assets/imgs_480_800/research/research_button_wall_down.png";
constexpr const char* kBowUp = "assets/imgs_480_800/research/research_button_bow_up.png";
constexpr const char* kBowDown = "assets/imgs_480_800/research/research_button_bow_down.png";
constexpr const char* kNumberStrip = "assets/imgs_480_800/game/z_number_list.png";
constexpr float kContinueX = 617.0f;
constexpr float kContinueY = 409.0f;
constexpr float kContinueW = 163.0f;
constexpr float kContinueH = 69.0f;
constexpr float kMoneyX = 460.0f;
constexpr float kMoneyY = 420.0f;
constexpr float kGoldYOffset = 50.0f;
constexpr float kStageBoxX = 16.0f;
constexpr float kStageBoxY = 420.0f;
constexpr float kStageBoxW = 160.0f;
constexpr float kStageBoxH = 40.0f;
constexpr float kStageFontSize = 24.0f;

struct MenuButton {
    const char* up = nullptr;
    const char* down = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

constexpr MenuButton kMenuButtons[4] = {
    {kDefenderUp, kDefenderDown, 2.0f, 316.0f, 78.0f, 74.0f},
    {kMagicUp, kMagicDown, 2.0f, 242.0f, 78.0f, 74.0f},
    {kWallUp, kWallDown, 2.0f, 168.0f, 78.0f, 74.0f},
    {kBowUp, kBowDown, 2.0f, 94.0f, 78.0f, 74.0f}
};

const char* continue_up_path() {
    return Param::language == 1 ? kContinueUpKr : kContinueUp;
}

const char* continue_down_path() {
    return Param::language == 1 ? kContinueDownKr : kContinueDown;
}

void draw_centered_text_in_box(
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
    const float pixel_height = sy(font_size);
    const float text_width = fonts.measure_text_width(font, text, pixel_height);
    const float draw_x = sx(box_x) + std::max(0.0f, (sx(box_w) - text_width) * 0.5f) + sx(2.0f);
    const float draw_y = sy(box_y + ((box_h - font_size) * 0.5f) + font_size);
    fonts.draw_text(font, text, draw_x + sx(1.0f), draw_y + sy(1.0f), pixel_height, 0.0f, 0.0f, 0.0f, a);
    fonts.draw_text(font, text, draw_x, draw_y, pixel_height, r, g, b, a);
}

} // namespace

ResearchScene::ResearchScene(TransitionRequest transition_cb)
    : transition_cb_(std::move(transition_cb)),
      equip_zone_([this]() { show_bow_menu(); }),
      item_zone_(
          &equip_zone_,
          [this]() {
              if (transition_cb_) {
                  transition_cb_(Param::SCENE_SHOP, 0);
              }
          },
          [this]() { bow_flash_flag_ = true; }
      ) {}

void ResearchScene::show_bow_menu() {
    set_menu(3);
}

void ResearchScene::set_menu(int menu_id) {
    if (menu_id < 0 || menu_id > 3) {
        return;
    }
    pressed_menu_ = menu_id;
    item_zone_.set_item_type(menu_id);
}

bool ResearchScene::touch(const TouchEvent& event) {
    if (HelpOverlay::touch(event, event.x1, event.y1)) {
        return true;
    }

    if (equip_zone_.touch(event, event.x1, event.y1, event.x2, event.y2)) {
        return true;
    }

    if (item_zone_.touch(event, event.x1, event.y1, event.x2, event.y2)) {
        if (event.action == TouchAction::Down) {
            continue_pressed_ = hit_box(kContinueX, kContinueY, kContinueW, kContinueH, event.x1, event.y1);
            shop_pressed_ = hit_box(460.0f, 415.0f, 152.0f, 55.0f, event.x1, event.y1);
            if (shop_pressed_ && transition_cb_) {
                transition_cb_(Param::SCENE_SHOP, 0);
            }
            for (int i = 0; i < 4; ++i) {
                const auto& btn = kMenuButtons[static_cast<size_t>(i)];
                if (hit_box(btn.x, btn.y, btn.w, btn.h, event.x1, event.y1) && i != pressed_menu_) {
                    set_menu(i);
                    break;
                }
            }
        } else if (event.action == TouchAction::Up) {
            if (continue_pressed_ && hit_box(kContinueX, kContinueY, kContinueW, kContinueH, event.x1, event.y1)) {
                AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
                if (transition_cb_) {
                    transition_cb_(Param::SCENE_MAIN_GAME, 0);
                }
            }
            continue_pressed_ = false;
            shop_pressed_ = false;
        }
        return true;
    }

    if (event.action == TouchAction::Up) {
        continue_pressed_ = false;
        shop_pressed_ = false;
    }
    return true;
}

void ResearchScene::draw() {
    if (!draw_texture(kResearchBg, 0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 1.0f)) {
        draw_quad(0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 0.10f, 0.12f, 0.10f, 1.0f);
    }

    item_zone_.draw();

    for (int i = 0; i < 4; ++i) {
        const auto& btn = kMenuButtons[static_cast<size_t>(i)];
        draw_texture(
            i == pressed_menu_ ? btn.down : btn.up,
            sx(btn.x),
            sy(btn.y),
            sx(btn.w),
            sy(btn.h),
            1.0f
        );
    }

    draw_texture(
        continue_pressed_ ? continue_down_path() : continue_up_path(),
        sx(kContinueX),
        sy(kContinueY),
        sx(kContinueW),
        sy(kContinueH),
        1.0f
    );

    equip_zone_.draw();

    const float money_x = sx(kMoneyX);
    const float money_y = sy(kMoneyY);
    const auto& gold_layout = research_layout_tweak(ResearchLayoutElement::GoldAmount);
    const float scale = 0.5f;
    draw_number_strip(kNumberStrip, std::max(0, show_stone_), money_x, money_y, scale, 0.0625f, 0.1367f, 0.2109f, 1.0f);
    draw_number_strip(
        kNumberStrip,
        std::max(0, show_gold_),
        money_x + sx(gold_layout.offset_x),
        money_y + sy(kGoldYOffset + gold_layout.offset_y),
        scale * gold_layout.scale,
        0.0625f,
        0.1367f,
        0.2109f,
        1.0f
    );

    auto& fonts = FontRenderer::instance();
    const auto& stage_layout = research_layout_tweak(ResearchLayoutElement::CurrentStage);
    const std::string title = Param::is_online_mode ? "Battle" : ("Stage " + std::to_string(std::max(1, Param::stage)));
    draw_centered_text_in_box(
        fonts,
        FontFaceId::Cooper,
        title,
        kStageBoxX + stage_layout.offset_x,
        kStageBoxY + stage_layout.offset_y,
        kStageBoxW,
        kStageBoxH,
        kStageFontSize * stage_layout.scale,
        0.98f,
        0.92f,
        0.75f,
        0.98f
    );

    HelpOverlay::draw();
}

void ResearchScene::update() {
    if (bow_flash_flag_) {
        set_menu(3);
        bow_flash_flag_ = false;
    }

    item_zone_.update();

    if (show_gold_ != Param::gold) {
        show_gold_ = ((Param::gold - show_gold_) / 8) + (Param::gold > show_gold_ ? 1 : -1) + show_gold_;
    }
    if (show_stone_ != Param::stone) {
        show_stone_ += ((Param::stone - show_stone_) / 10) + (Param::stone > show_stone_ ? 1 : -1);
    }
}

void ResearchScene::reset() {
    Param::gold = Save::load_data(Save::GOLD);
    Param::stone = Save::load_data(Save::STONE);
    ItemParam::load_level();

    continue_pressed_ = false;
    shop_pressed_ = false;
    bow_flash_flag_ = false;
    pressed_menu_ = 0;

    show_gold_ = Param::gold;
    show_stone_ = Param::stone;

    item_zone_.reset();
    equip_zone_.reset();
    set_menu(0);

    scene_x = 0.0f;
    scene_y = 0.0f;
    scene_scale = 1.0f;
    scene_alpha = 1.0f;
}

} // namespace defender
