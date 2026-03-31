#include "scenes/main_game_scene.hpp"

#include <algorithm>

#include "engine/abstract_game.hpp"
#include "game/param.hpp"

namespace defender {
namespace {

constexpr int kRandomForm = 0;
constexpr int kRowForm = 1;
constexpr int kOneForm = 2;
constexpr int kGooseForm = 3;

}

int MainGameScene::random_stage_monster_type() {
    int max_type = 0;
    if (Param::stage > 3) {
        max_type = 1;
    }
    if (Param::stage > 10) {
        max_type = 2;
    }
    if (Param::stage > 20) {
        max_type = 3;
    }
    return static_cast<int>(form_rng_() % static_cast<unsigned>(max_type + 1));
}

int MainGameScene::goose_form(int number, int mons_type, int check_time) {
    if (number < 8) {
        spawn_monster(false, mons_type, check_time, 260);
        const int p = number / 2;
        for (int i = 0; i < p; ++i) {
            spawn_monster(false, mons_type, (((i + 1) * 600) / 2) + check_time, ((i + 1) * 50) + 260);
            spawn_monster(false, mons_type, (((i + 1) * 600) / 2) + check_time, 260 - ((i + 1) * 50));
        }
        return 1500;
    }

    int temp = 0;
    while (number > 7) {
        goose_form(7, mons_type, temp * 600);
        ++temp;
        number -= 7;
    }
    goose_form(number, mons_type, temp * 600);
    return 1500 + (temp * 600);
}

int MainGameScene::one_form(int number, int mons_type, int check_time) {
    if (number < 8) {
        spawn_monster(false, mons_type, check_time, 260);
        const int p = number / 2;
        for (int i = 0; i < p; ++i) {
            spawn_monster(false, mons_type, check_time, ((i + 1) * 50) + 260);
            spawn_monster(false, mons_type, check_time, 260 - ((i + 1) * 50));
        }
        return 600;
    }

    int temp = 0;
    while (number > 7) {
        one_form(7, mons_type, temp * 600);
        number -= 7;
        ++temp;
    }
    one_form(number, mons_type, temp * 600);
    return 600 + (temp * 600);
}

int MainGameScene::row_form(int number, int mons_type, int check_time) {
    if (number < 6) {
        for (int i = 0; i < number; ++i) {
            spawn_monster(false, mons_type, (i * 600) + check_time, 260);
        }
        return number * 600;
    }
    if (number < 11) {
        for (int i = 0; i < number / 2; ++i) {
            spawn_monster(false, mons_type, (i * 600) + check_time, 310);
            spawn_monster(false, mons_type, (i * 600) + check_time, 210);
        }
        return (number / 2) * 600;
    }
    if (number < 16) {
        for (int i = 0; i < number / 3; ++i) {
            spawn_monster(false, mons_type, (i * 600) + check_time, 360);
            spawn_monster(false, mons_type, (i * 600) + check_time, 260);
            spawn_monster(false, mons_type, (i * 600) + check_time, 160);
        }
        return (number / 3) * 600;
    }

    for (int i = 0; i < number / 4; ++i) {
        spawn_monster(false, mons_type, (i * 600) + check_time, 410);
        spawn_monster(false, mons_type, (i * 600) + check_time, 310);
        spawn_monster(false, mons_type, (i * 600) + check_time, 210);
        spawn_monster(false, mons_type, (i * 600) + check_time, 110);
    }
    return (number / 4) * 600;
}

int MainGameScene::add_formation(int form_type, int number, int mons_type, int check_time) {
    switch (form_type) {
    case kRandomForm:
        return add_formation(static_cast<int>(form_rng_() % 3) + 1, number, mons_type, check_time);
    case kRowForm:
        return row_form(number, mons_type, check_time);
    case kOneForm:
        return one_form(number, mons_type, check_time);
    case kGooseForm:
        return goose_form(number, mons_type, check_time);
    default:
        return 0;
    }
}

void MainGameScene::spawn_stage_bosses() {
    if (Param::stage % 50 == 0) {
        if (Param::stage == 50) {
            spawn_monster(true, 5, 16000, -10);
        } else if (Param::stage == 100) {
            spawn_monster(true, 5, 10000, -10);
            spawn_monster(true, 4, 16000, -10);
        } else if (Param::stage >= 150) {
            spawn_monster(true, 5, 10000, -10);
            spawn_monster(true, 4, 16000, -10);
            spawn_monster(true, 4, 16000, -10);
        }
        return;
    }

    if (Param::stage % 10 != 0) {
        return;
    }
    if (Param::stage < 50) {
        spawn_monster(true, 4, 16000, -10);
    } else if (Param::stage < 100) {
        spawn_monster(true, 4, 16000, -10);
        spawn_monster(true, 4, 16000, -10);
    } else if (Param::stage < 150) {
        spawn_monster(true, 5, 10000, -10);
        spawn_monster(true, 4, 16000, -10);
    } else if (Param::stage > 150) {
        spawn_monster(true, 5, 10000, -10);
        spawn_monster(true, 4, 16000, -10);
        spawn_monster(true, 4, 16000, -10);
    }
}

void MainGameScene::configure_stage() {
    section_monster_num_ = {
        0,
        ((Param::stage + 3) / 7) + 1,
        ((Param::stage + 1) / 6) + 1,
        ((Param::stage + 2) / 6) + 2,
        ((Param::stage + 6) / 8) + 1,
        ((Param::stage + 5) / 7) + 1,
        ((Param::stage + 4) / 6) + 1,
        ((Param::stage + 5) / 6) + 2
    };

    section_time_sec_ = {
        1,
        10,
        20,
        (Param::stage / 30) + 23,
        (Param::stage / 30) + 30,
        (Param::stage / 30) + 40,
        (Param::stage / 30) + 50,
        (Param::stage / 15) + 53
    };

    section_spawned_.fill(0);
    mission_section_point_ = 1;
    active_section_ = 1;
    stage_total_time_sec_ = section_time_sec_[7];
    spawn_frequency_ms_ = std::max(200, 3000 - (Param::stage * 15));
    no_form_freq_ms_ = spawn_frequency_ms_;
    no_form_time_ms_ = 0;
    no_form_type_ = -10;
    form_lag_time_ms_ = 1000;
    total_spawn_target_ = 0;
    total_spawned_ = 0;
    boss_killed_ = (Param::stage % 10 != 0);
    boss_spawned_ = boss_killed_;
}

void MainGameScene::update_wave() {
    if (is_online_mode_) {
        return;
    }

    const int game_time_ms = static_cast<int>(AbstractGame::game_time_ms());
    if (game_time_ms > form_lag_time_ms_ && !is_game_finish_) {
        if (no_form_time_ms_ <= 0) {
            int monster_type = no_form_type_;
            if (monster_type == -10) {
                monster_type = random_stage_monster_type();
                if (Param::stage > 50 && static_cast<int>(form_rng_() % 1000) < Param::stage / 10) {
                    monster_type = 6;
                }
            }
            spawn_monster(false, monster_type, 0, -10);
            no_form_time_ms_ += no_form_freq_ms_ + static_cast<int>(form_rng_() % 1000);
        } else {
            no_form_time_ms_ -= static_cast<int>(AbstractGame::last_span_ms());
        }
    }

    if (mission_section_point_ >= static_cast<int>(section_time_sec_.size())) {
        if (!boss_spawned_) {
            spawn_stage_bosses();
            boss_spawned_ = true;
        }
        return;
    }

    const int section_start_ms = section_time_sec_[static_cast<size_t>(mission_section_point_)] * 1000;
    if (game_time_ms <= section_start_ms || game_time_ms <= form_lag_time_ms_) {
        return;
    }

    int monster_type = random_stage_monster_type();
    form_lag_time_ms_ = add_formation(0, section_monster_num_[static_cast<size_t>(mission_section_point_)], monster_type, 0) + section_start_ms;

    if (Param::stage > 30) {
        const int fix = Param::stage - 30;
        const int rate = static_cast<int>(form_rng_() % 1000);
        const int wait_time = form_lag_time_ms_ - section_start_ms;
        if (mission_section_point_ == 1) {
            if (rate < static_cast<int>((fix - 20) * 1.1f)) {
                spawn_monster(false, 6, wait_time + 1000, 350);
                spawn_monster(false, 6, wait_time + 1000, 150);
                spawn_monster(false, 6, wait_time, 250);
            } else if (rate < (fix - 20) * 3) {
                spawn_monster(false, 6, wait_time, 200);
                spawn_monster(false, 6, wait_time, 300);
            } else if (rate < fix * 5) {
                spawn_monster(false, 6, wait_time, 250);
            }
        }
        if (mission_section_point_ == 4) {
            if (rate < static_cast<int>((fix - 20) * 1.3f)) {
                spawn_monster(false, 6, wait_time + 1000, 340);
                spawn_monster(false, 6, wait_time + 1000, 140);
                spawn_monster(false, 6, wait_time, 240);
            } else if (rate < (fix - 20) * 4) {
                spawn_monster(false, 6, wait_time, 200);
                spawn_monster(false, 6, wait_time, 300);
            } else if (rate < (fix * 5) + 400 && Param::stage > 50) {
                spawn_monster(false, 6, wait_time, 250);
            }
        }
        if (mission_section_point_ == 7 && Param::stage % 10 != 0) {
            if (rate < static_cast<int>((fix - 20) * 1.2f)) {
                spawn_monster(false, 6, wait_time + 1000, 340);
                spawn_monster(false, 6, wait_time + 1000, 140);
                spawn_monster(false, 6, wait_time, 240);
            } else if (rate < static_cast<int>((fix - 20) * 3.5f)) {
                spawn_monster(false, 6, wait_time, 200);
                spawn_monster(false, 6, wait_time, 300);
            } else if (rate < (fix * 5) + 400 && Param::stage > 50) {
                spawn_monster(false, 6, wait_time, 250);
            }
        }
    }

    ++mission_section_point_;
}

} // namespace defender
