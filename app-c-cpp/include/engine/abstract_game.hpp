#pragma once

#include <cstdint>

namespace defender {

class AbstractGame {
public:
    virtual ~AbstractGame() = default;

    virtual void draw_frame() = 0;
    virtual void update_frame();

    static void pause();
    static void resume();
    static bool is_paused();

    static uint64_t game_time_ms();
    static uint64_t last_span_ms();

    static void reset_game_time();
    static void reset_game_time(uint64_t time_ms);

private:
    bool game_started_ = false;

    static bool is_game_paused_;
    static uint64_t last_game_time_ms_;
    static uint64_t prev_mark_ms_;
    static uint64_t total_game_time_ms_;
};

}

