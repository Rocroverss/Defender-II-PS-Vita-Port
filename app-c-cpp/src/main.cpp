#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

#include <psp2/ctrl.h>
#include <psp2/ime.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/touch.h>
#include <vitaGL.h>

#include "engine/abstract_game.hpp"
#include "engine/bmp_scaler.hpp"
#include "engine/coordinate_mapper.hpp"
#include "engine/screen.hpp"
#include "engine/touch_event.hpp"
#include "game/game.hpp"
#include "game/param.hpp"
#include "game/save.hpp"
#include "game/umeng_helper.hpp"
#include "scenes/scene.hpp"

using namespace defender;

namespace {

constexpr int kDefaultViewportWidth = 960;
constexpr int kDefaultViewportHeight = 544;
constexpr float kTouchMaxX = 1919.0f;
constexpr float kTouchMaxY = 1087.0f;
constexpr int kImeMaxChars = 32;
constexpr float kStickDeadZone = 32.0f;

int g_viewport_width = kDefaultViewportWidth;
int g_viewport_height = kDefaultViewportHeight;

struct ImeState {
    bool module_loaded = false;
    bool open = false;
    bool close_requested = false;
    bool submitted = false;
    uint8_t work[SCE_IME_WORK_BUFFER_SIZE] = {};
    SceWChar16 initial[kImeMaxChars + 1] = {};
    SceWChar16 input[kImeMaxChars + 1] = {};
    std::string submitted_text;
};

ImeState g_ime;

void ascii_to_utf16(const std::string& text, SceWChar16* out, size_t cap) {
    if (out == nullptr || cap == 0) {
        return;
    }
    const size_t n = std::min(cap - 1, text.size());
    for (size_t i = 0; i < n; ++i) {
        out[i] = static_cast<SceWChar16>(static_cast<unsigned char>(text[i]));
    }
    out[n] = 0;
}

std::string utf16_to_ascii(const SceWChar16* in, size_t cap) {
    if (in == nullptr || cap == 0) {
        return {};
    }
    std::string out;
    out.reserve(cap);
    for (size_t i = 0; i < cap; ++i) {
        const SceWChar16 c = in[i];
        if (c == 0) {
            break;
        }
        if (c >= 32 && c <= 126) {
            out.push_back(static_cast<char>(c));
        }
    }
    return out;
}

void ime_event_handler(void* arg, const SceImeEventData* e) {
    auto* ime = static_cast<ImeState*>(arg);
    if (ime == nullptr || e == nullptr) {
        return;
    }
    if (e->id == SCE_IME_EVENT_PRESS_ENTER) {
        ime->submitted_text = utf16_to_ascii(ime->input, kImeMaxChars + 1);
        ime->submitted = true;
        ime->close_requested = true;
    } else if (e->id == SCE_IME_EVENT_PRESS_CLOSE) {
        ime->close_requested = true;
    }
}

bool ime_open_name_input(const std::string& current_name) {
    if (g_ime.open) {
        return false;
    }
    if (!g_ime.module_loaded) {
        const int load_res = sceSysmoduleLoadModule(SCE_SYSMODULE_IME);
        if (load_res < 0) {
            return false;
        }
        g_ime.module_loaded = true;
    }

    std::memset(g_ime.work, 0, sizeof(g_ime.work));
    std::memset(g_ime.initial, 0, sizeof(g_ime.initial));
    std::memset(g_ime.input, 0, sizeof(g_ime.input));
    g_ime.close_requested = false;
    g_ime.submitted = false;
    g_ime.submitted_text.clear();
    ascii_to_utf16(current_name, g_ime.initial, kImeMaxChars + 1);
    ascii_to_utf16(current_name, g_ime.input, kImeMaxChars + 1);

    SceImeParam param;
    sceImeParamInit(&param);
    param.supportedLanguages = SCE_IME_LANGUAGE_ENGLISH | SCE_IME_LANGUAGE_SPANISH;
    param.languagesForced = SCE_FALSE;
    param.type = SCE_IME_TYPE_DEFAULT;
    param.option = SCE_IME_OPTION_NO_AUTO_CAPITALIZATION;
    param.work = g_ime.work;
    param.arg = &g_ime;
    param.handler = ime_event_handler;
    param.initialText = g_ime.initial;
    param.maxTextLength = kImeMaxChars;
    param.inputTextBuffer = g_ime.input;
    param.enterLabel = SCE_IME_ENTER_LABEL_GO;

    const int open_res = sceImeOpen(&param);
    if (open_res < 0) {
        return false;
    }
    g_ime.open = true;
    return true;
}

void ime_update() {
    if (!g_ime.open) {
        return;
    }

    sceImeUpdate();

    if (g_ime.close_requested) {
        sceImeClose();
        g_ime.open = false;
        g_ime.close_requested = false;
        if (g_ime.submitted && !g_ime.submitted_text.empty()) {
            Param::player_name = g_ime.submitted_text;
            Save::save_name(g_ime.submitted_text);
        }
        g_ime.submitted = false;
    }
}

void ime_shutdown() {
    if (g_ime.open) {
        sceImeClose();
        g_ime.open = false;
    }
    if (g_ime.module_loaded) {
        sceSysmoduleUnloadModule(SCE_SYSMODULE_IME);
        g_ime.module_loaded = false;
    }
}

TouchEvent to_touch_event(const SceTouchReport& report, TouchAction action) {
    TouchEvent ev;
    ev.action = action;
    ev.pointer_count = 1;
    ev.x1 = static_cast<float>(report.x) * (static_cast<float>(g_viewport_width) / kTouchMaxX);
    ev.y1 = static_cast<float>(g_viewport_height) - (static_cast<float>(report.y) * (static_cast<float>(g_viewport_height) / kTouchMaxY));
    return ev;
}

void dispatch_tap(Game& game, float x, float y) {
    TouchEvent down;
    down.action = TouchAction::Down;
    down.pointer_count = 1;
    down.x1 = x;
    down.y1 = y;
    game.touch(down);

    TouchEvent up = down;
    up.action = TouchAction::Up;
    game.touch(up);
}

bool stick_to_bow_target(const SceCtrlData& pad, float* out_x, float* out_y) {
    if (out_x == nullptr || out_y == nullptr) {
        return false;
    }

    auto normalized_axis = [](uint8_t axis, bool invert) -> float {
        const float value = (static_cast<float>(axis) - 127.5f) / 127.5f;
        return invert ? -value : value;
    };

    float dx = normalized_axis(pad.rx, false);
    float dy = normalized_axis(pad.ry, true);
    float magnitude = std::sqrt((dx * dx) + (dy * dy));

    if (magnitude * 127.5f < kStickDeadZone) {
        dx = normalized_axis(pad.lx, false);
        dy = normalized_axis(pad.ly, true);
        magnitude = std::sqrt((dx * dx) + (dy * dy));
    }

    if (magnitude * 127.5f < kStickDeadZone || magnitude <= 0.001f) {
        return false;
    }

    dx /= magnitude;
    dy /= magnitude;

    const float bow_x = Scene::get_x(10.0f);
    const float bow_y = Scene::get_y(240.0f);
    const float radius_x = Scene::get_x(760.0f);
    const float radius_y = Scene::get_y(220.0f);

    *out_x = std::clamp(
        bow_x + (dx * radius_x),
        bow_x + Scene::get_x(24.0f),
        Scene::screen_width - Scene::get_x(24.0f)
    );
    *out_y = std::clamp(
        bow_y + (dy * radius_y),
        Scene::get_y(72.0f),
        Scene::screen_height - Scene::get_y(108.0f)
    );
    return true;
}

void init_gl_state() {
    glClearDepthf(1.0f);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void begin_frame() {
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, g_viewport_width, g_viewport_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0.0f, static_cast<float>(g_viewport_width), 0.0f, static_cast<float>(g_viewport_height), -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void resolve_runtime_viewport() {
    // Ask GL for the viewport it can really use; some runtimes clamp to 800x480.
    glViewport(0, 0, kDefaultViewportWidth, kDefaultViewportHeight);
    GLint vp[4] = {0, 0, kDefaultViewportWidth, kDefaultViewportHeight};
    glGetIntegerv(GL_VIEWPORT, vp);
    if (vp[2] > 0 && vp[3] > 0) {
        g_viewport_width = vp[2];
        g_viewport_height = vp[3];
    } else {
        g_viewport_width = kDefaultViewportWidth;
        g_viewport_height = kDefaultViewportHeight;
    }
}

void prime_screen_scalers() {
    auto& screen = Screen::current();
    screen.set_bounds(g_viewport_width, g_viewport_height);

    auto& mapper = CoordinateMapper::instance();
    mapper.set_designed(static_cast<float>(Param::DESIGNED_SCREEN_WIDTH), static_cast<float>(Param::DESIGNED_SCREEN_HEIGHT));

    auto& bmp = BmpScaler::instance();
    bmp.set_bitmap_original(static_cast<float>(Param::DESIGNED_SCREEN_WIDTH), static_cast<float>(Param::DESIGNED_SCREEN_HEIGHT));

    Scene::scene_init();
}

void handle_back_action(Game& game) {
    // GameActivity.onKeyDown(BACK) parity (scene-based routing).
    if (game.current_scene() != game.target_scene()) {
        return;
    }

    switch (game.current_scene()) {
    case Game::MAIN_GAME:
        if (game.is_main_game_over()) {
            break;
        }
        if (AbstractGame::is_paused()) {
            AbstractGame::resume();
        } else {
            AbstractGame::pause();
        }
        break;
    case Game::RESEARCH:
        game.tran_scene(Game::ONLINE_DATA, Game::TRANS_FROM_LEFT);
        break;
    case Game::STATS:
        game.tran_scene(Game::RESEARCH, Game::TRANS_FROM_LEFT);
        break;
    case Game::SHOP:
        game.tran_scene(Game::RESEARCH, Game::TRANS_FROM_LEFT);
        break;
    case Game::ONLINE_DATA:
        game.tran_scene(Game::COVER, Game::TRANS_FROM_LEFT);
        break;
    case Game::COVER:
    case Game::LOADING:
    default:
        // Android opens featured dialog here; no Vita counterpart yet.
        break;
    }
}

void on_create_resume() {
    Param::is_create = true;
    AbstractGame::resume();
    UMengHelper::on_resume();
}

void on_pause_for_exit() {
    Param::is_create = false;
    AbstractGame::pause();
    UMengHelper::on_pause();

    Save::pause_save_data();
    Save::save_data(Save::LEVEL, Param::level);
    Save::save_data(Save::XP, Param::xp);
    Save::save_data(Save::WIN, Param::win);
    Save::save_data(Save::LOSE, Param::lose);
    Save::save_data(Save::STAGE, Param::stage);
    Save::save_data(Save::FIRE_CAST, Param::cast_fire);
    Save::save_data(Save::ICE_CAST, Param::cast_ice);
    Save::save_data(Save::LIGHT_CAST, Param::cast_light);
    Save::save_data(Save::KILLS, Param::total_kills);
    Save::save_data(Save::COST_COIN, Param::cost_coin);
    Save::save_data(Save::COST_STONE, Param::cost_stone);
}

}

int main() {
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

    vglInit(8 * 1024 * 1024);
    init_gl_state();
    resolve_runtime_viewport();
    prime_screen_scalers();

    Game game;
    game.initialize(g_viewport_width, g_viewport_height);
    on_create_resume();

    SceCtrlData pad{};
    uint32_t prev_buttons = 0;
    bool had_touch = false;
    SceTouchReport last_report{};
    bool had_stick_aim = false;
    float last_stick_x = 0.0f;
    float last_stick_y = 0.0f;

    bool running = true;
    while (running) {
        if (Param::request_name_edit) {
            Param::request_name_edit = false;
            ime_open_name_input(Param::player_name);
        }
        ime_update();

        sceCtrlPeekBufferPositive(0, &pad, 1);

        const uint32_t pressed = (pad.buttons ^ prev_buttons) & pad.buttons;
        prev_buttons = pad.buttons;

        // Vita-only emergency exit combo.
        if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_SELECT) &&
            ((pressed & SCE_CTRL_START) || (pressed & SCE_CTRL_SELECT))) {
            running = false;
        }

        // START behaves as Android BACK.
        if (pressed & SCE_CTRL_START) {
            handle_back_action(game);
        }

        if (pressed & SCE_CTRL_TRIANGLE) {
            dispatch_tap(game, 560.0f, 414.0f); // Fire spell button
        }
        if (pressed & SCE_CTRL_CIRCLE) {
            dispatch_tap(game, 740.0f, 414.0f); // Lightning spell button
        }
        if (pressed & SCE_CTRL_SQUARE) {
            dispatch_tap(game, 650.0f, 414.0f); // Ice spell button
        }
        if (pressed & SCE_CTRL_RTRIGGER) {
            dispatch_tap(game, 65.0f, 400.0f); // Add mana button
        }

        SceTouchData touch_data{};
        std::memset(&touch_data, 0, sizeof(touch_data));
        sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch_data, 1);
        if (touch_data.reportNum > 0) {
            const auto& report = touch_data.report[0];
            TouchEvent ev = to_touch_event(report, had_touch ? TouchAction::Move : TouchAction::Down);
            game.touch(ev);
            last_report = report;
            had_touch = true;
        } else if (had_touch) {
            TouchEvent ev = to_touch_event(last_report, TouchAction::Up);
            game.touch(ev);
            had_touch = false;
        }

        const bool allow_stick_aim =
            game.current_scene() == Game::MAIN_GAME &&
            game.target_scene() == Game::MAIN_GAME &&
            !game.is_main_game_over() &&
            !AbstractGame::is_paused() &&
            !had_touch;
        float stick_target_x = 0.0f;
        float stick_target_y = 0.0f;
        if (allow_stick_aim && stick_to_bow_target(pad, &stick_target_x, &stick_target_y)) {
            TouchEvent ev;
            ev.pointer_count = 1;
            ev.x1 = stick_target_x;
            ev.y1 = stick_target_y;
            ev.action = had_stick_aim ? TouchAction::Move : TouchAction::Down;
            game.touch(ev);
            last_stick_x = stick_target_x;
            last_stick_y = stick_target_y;
            had_stick_aim = true;
        } else if (had_stick_aim) {
            TouchEvent ev;
            ev.action = TouchAction::Up;
            ev.pointer_count = 1;
            ev.x1 = last_stick_x;
            ev.y1 = last_stick_y;
            game.touch(ev);
            had_stick_aim = false;
        }

        begin_frame();
        game.draw_frame();

        vglSwapBuffers(GL_FALSE);
    }

    on_pause_for_exit();
    ime_shutdown();
    vglEnd();
    sceKernelExitProcess(0);
    return 0;
}
