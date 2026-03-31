#include "engine/abstract_game.hpp"

#include "engine/time_utils.hpp"

namespace defender {

bool AbstractGame::is_game_paused_ = false;
uint64_t AbstractGame::last_game_time_ms_ = 0;
uint64_t AbstractGame::prev_mark_ms_ = 0;
uint64_t AbstractGame::total_game_time_ms_ = 0;

void AbstractGame::update_frame() {
    if (!game_started_) {
        prev_mark_ms_ = now_ms();
        total_game_time_ms_ = 0;
        game_started_ = true;
    }

    if (!is_game_paused_) {
        last_game_time_ms_ = total_game_time_ms_;
        const uint64_t now = now_ms();
        uint64_t delta = now - prev_mark_ms_;
        if (delta > 50ULL) {
            delta = 50ULL;
        }
        total_game_time_ms_ += delta;
        prev_mark_ms_ = now;
    }
}

void AbstractGame::pause() {
    is_game_paused_ = true;
}

void AbstractGame::resume() {
    is_game_paused_ = false;
    prev_mark_ms_ = now_ms();
}

bool AbstractGame::is_paused() {
    return is_game_paused_;
}

uint64_t AbstractGame::game_time_ms() {
    return total_game_time_ms_;
}

uint64_t AbstractGame::last_span_ms() {
    if (total_game_time_ms_ < last_game_time_ms_) {
        return 0;
    }
    return total_game_time_ms_ - last_game_time_ms_;
}

void AbstractGame::reset_game_time() {
    total_game_time_ms_ = 0;
    last_game_time_ms_ = 0;
    prev_mark_ms_ = now_ms();
}

void AbstractGame::reset_game_time(uint64_t time_ms) {
    total_game_time_ms_ = time_ms;
    last_game_time_ms_ = time_ms;
    prev_mark_ms_ = now_ms();
}

}

