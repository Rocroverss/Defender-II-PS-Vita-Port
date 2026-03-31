#include "scenes/cover_scene.hpp"

#include <utility>

#include "engine/abstract_game.hpp"
#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"
#include "game/audio_manager.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "game/sounds.hpp"

namespace defender {

namespace {

constexpr const char* kCoverBg = "assets/imgs_480_800/cover/cover_fixed.jpg";

constexpr const char* kMusicOn = "assets/imgs_480_800/cover/zzz_button_music_on.png";
constexpr const char* kMusicOff = "assets/imgs_480_800/cover/zzz_button_music_off.png";
constexpr const char* kSoundOn = "assets/imgs_480_800/cover/button_sound_on.png";
constexpr const char* kSoundOff = "assets/imgs_480_800/cover/button_sound_off.png";

constexpr const char* kHonorUp = "assets/imgs_480_800/cover/cover_button_honor_up.png";
constexpr const char* kHonorDown = "assets/imgs_480_800/cover/cover_button_honor_down.png";
constexpr const char* kMoreUp = "assets/imgs_480_800/cover/cover_button_more_up.png";
constexpr const char* kMoreDown = "assets/imgs_480_800/cover/cover_button_more_down.png";
constexpr const char* kStartUp = "assets/imgs_480_800/cover/cover_button_start_up.png";
constexpr const char* kStartDown = "assets/imgs_480_800/cover/cover_button_start_down.png";

constexpr float kMusicX = 25.0f;
constexpr float kMusicY = 208.0f;
constexpr float kMusicW = 43.0f;
constexpr float kMusicH = 38.0f;

constexpr float kSoundX = 66.0f;
constexpr float kSoundY = 156.0f;
constexpr float kSoundW = 44.0f;
constexpr float kSoundH = 40.0f;

constexpr float kStartX = 654.0f;
constexpr float kStartY = 230.0f;
constexpr float kStartW = 150.0f;
constexpr float kStartH = 80.0f;

constexpr float kHonorX = 607.0f;
constexpr float kHonorY = 134.0f;
constexpr float kHonorW = 150.0f;
constexpr float kHonorH = 86.0f;

constexpr float kMoreX = 560.0f;
constexpr float kMoreY = 32.0f;
constexpr float kMoreW = 150.0f;
constexpr float kMoreH = 95.0f;

void draw_background_cover(const char* path) {
    const auto& tex = TextureCache::instance().get(path);
    if (tex.valid) {
        draw_textured_quad(tex.id, 0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 1.0f);
    } else {
        draw_quad(0.0f, 0.0f, Scene::screen_width, Scene::screen_height, 0.08f, 0.10f, 0.14f, 1.0f);
    }
}

} // namespace

CoverScene::CoverScene(TransitionRequest transition_cb) : transition_cb_(std::move(transition_cb)) {}

bool CoverScene::hit_test(const char* path,
                          float button_x,
                          float button_y,
                          float fallback_w,
                          float fallback_h,
                          float point_x,
                          float point_y,
                          float pad_x,
                          float pad_y) {
    const auto& tex = TextureCache::instance().get(path);
    const float w = fallback_w > 0.0f ? fallback_w : (tex.valid ? static_cast<float>(tex.width) : 0.0f);
    const float h = fallback_h > 0.0f ? fallback_h : (tex.valid ? static_cast<float>(tex.height) : 0.0f);

    const float x0 = Scene::get_x(button_x - pad_x);
    const float y0 = Scene::get_y(button_y - pad_y);
    const float x1 = Scene::get_x(button_x + w + pad_x);
    const float y1 = Scene::get_y(button_y + h + pad_y);
    return point_x >= x0 && point_x <= x1 && point_y >= y0 && point_y <= y1;
}

void CoverScene::draw_sprite(const char* path,
                             float x,
                             float y,
                             float fallback_w,
                             float fallback_h,
                             float alpha) {
    const auto& tex = TextureCache::instance().get(path);
    const float base_w = fallback_w > 0.0f ? fallback_w : (tex.valid ? static_cast<float>(tex.width) : 0.0f);
    const float base_h = fallback_h > 0.0f ? fallback_h : (tex.valid ? static_cast<float>(tex.height) : 0.0f);
    const float draw_x = get_x(x);
    const float draw_y = get_y(y);
    const float draw_w = get_x(base_w);
    const float draw_h = get_y(base_h);

    if (tex.valid) {
        draw_textured_quad(tex.id, draw_x, draw_y, draw_w, draw_h, alpha);
    } else {
        draw_quad(draw_x, draw_y, draw_w, draw_h, 0.2f, 0.2f, 0.2f, alpha);
    }
}

void CoverScene::draw_button(const char* up_path,
                             const char* down_path,
                             float x,
                             float y,
                             float fallback_w,
                             float fallback_h,
                             bool pressed,
                             float alpha) {
    draw_sprite(pressed ? down_path : up_path, x, y, fallback_w, fallback_h, alpha);
}

bool CoverScene::touch(const TouchEvent& event) {
    if (event.action == TouchAction::Down) {
        pressed_button_ = PressedButton::None;

        if (hit_test(kMusicOn, kMusicX, kMusicY, kMusicW, kMusicH, event.x1, event.y1, 20.0f, 20.0f)) {
            Param::music_flag = !Param::music_flag;
            Save::save_data(Save::MUSIC_FLAG, Param::music_flag ? 0 : 1, Save::GLOBAL_DATA);
            AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
            return true;
        }
        if (hit_test(kSoundOn, kSoundX, kSoundY, kSoundW, kSoundH, event.x1, event.y1, 40.0f, 40.0f)) {
            Param::sound_effect_flag = !Param::sound_effect_flag;
            Save::save_data(Save::SOUND_FLAG, Param::sound_effect_flag ? 0 : 1, Save::GLOBAL_DATA);
            AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
            return true;
        }

        if (hit_test(kStartUp, kStartX, kStartY, kStartW, kStartH, event.x1, event.y1)) {
            pressed_button_ = PressedButton::Start;
            return true;
        }
        if (hit_test(kHonorUp, kHonorX, kHonorY, kHonorW, kHonorH, event.x1, event.y1)) {
            pressed_button_ = PressedButton::Honor;
            return true;
        }
        if (hit_test(kMoreUp, kMoreX, kMoreY, kMoreW, kMoreH, event.x1, event.y1)) {
            pressed_button_ = PressedButton::More;
            return true;
        }
        return true;
    }

    if (event.action == TouchAction::Up) {
        const PressedButton released = pressed_button_;
        pressed_button_ = PressedButton::None;

        if (released == PressedButton::Start &&
            hit_test(kStartUp, kStartX, kStartY, kStartW, kStartH, event.x1, event.y1)) {
            AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
            Param::is_online_mode = false;
            AbstractGame::reset_game_time();
            if (transition_cb_) {
                transition_cb_(Param::SCENE_ONLINE_DATA, 0);
            }
        }
        if (released == PressedButton::More &&
            hit_test(kMoreUp, kMoreX, kMoreY, kMoreW, kMoreH, event.x1, event.y1)) {
            AudioManager::instance().play_sound(Sounds::BUTTON_CLICK_SND);
            if (transition_cb_) {
                transition_cb_(Param::SCENE_SHOP, 0);
            }
        }
        return true;
    }

    if (event.action == TouchAction::Move && pressed_button_ != PressedButton::None) {
        return true;
    }

    return true;
}

void CoverScene::draw() {
    draw_background_cover(kCoverBg);

    draw_button(
        kHonorUp, kHonorDown, kHonorX, kHonorY, kHonorW, kHonorH, pressed_button_ == PressedButton::Honor, 1.0f
    );
    draw_button(
        kMoreUp, kMoreDown, kMoreX, kMoreY, kMoreW, kMoreH, pressed_button_ == PressedButton::More, more_highlight_alpha_
    );
    draw_button(
        kStartUp, kStartDown, kStartX, kStartY, kStartW, kStartH, pressed_button_ == PressedButton::Start, 1.0f
    );

    draw_sprite(Param::sound_effect_flag ? kSoundOn : kSoundOff, kSoundX, kSoundY, kSoundW, kSoundH, 1.0f);
    draw_sprite(Param::music_flag ? kMusicOn : kMusicOff, kMusicX, kMusicY, kMusicW, kMusicH, 1.0f);
}

void CoverScene::update() {
    const float delta = static_cast<float>(AbstractGame::last_span_ms()) / 650.0f;
    if (more_highlight_increasing_) {
        more_highlight_alpha_ += delta;
        if (more_highlight_alpha_ >= 1.0f) {
            more_highlight_alpha_ = 1.0f;
            more_highlight_increasing_ = false;
        }
    } else {
        more_highlight_alpha_ -= delta;
        if (more_highlight_alpha_ <= 0.55f) {
            more_highlight_alpha_ = 0.55f;
            more_highlight_increasing_ = true;
        }
    }
}

void CoverScene::reset() {
    pressed_button_ = PressedButton::None;
    more_highlight_alpha_ = 1.0f;
    more_highlight_increasing_ = false;
    scene_x = 0.0f;
    scene_y = 0.0f;
    scene_scale = 1.0f;
    scene_alpha = 1.0f;
}

} // namespace defender
