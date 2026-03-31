#pragma once

#include <utility>

namespace defender {

class AchvMng {
public:
    static void init();

    static void check_achv_in_game(int type);
    static void push_stage_score_in_game(int stage);
    static void push_battle_win_in_game(int win);

    static void sign_in();
    static void sign_out();
    static bool is_sign_in();
    static void show_achv_board();
    static void show_stage_board();
    static void push_achv_on_sign_in();

    static void update();

    // Returns true when a new achievement unlock is available.
    static bool pop_unlocked(int& type, int& level);

private:
    static int calc_amount_for_type(int type);
    static int calc_level_for_type(int type, int amount);
};

} // namespace defender

