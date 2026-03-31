#include "scenes/main_game_scene.hpp"

namespace defender {

void MainGameScene::update_mission_state() {
    if (mission_section_point_ >= static_cast<int>(section_time_sec_.size()) &&
        !is_game_finish_) {
        is_game_finish_ = true;
        gameover_time_ms_ = 0;
    }
}

}
