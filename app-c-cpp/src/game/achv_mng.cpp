#include "game/achv_mng.hpp"

#include <array>
#include <deque>

#include "game/achv_data.hpp"
#include "game/param.hpp"
#include "game/save.hpp"

namespace defender {
namespace {

struct UnlockEntry {
    int type = 0;
    int level = 0;
};

std::array<int, 8> s_achv_lv = {};
std::deque<UnlockEntry> s_pending_unlocks;
bool s_signed_in = false;

} // namespace

void AchvMng::init() {
    for (int type = 0; type < static_cast<int>(s_achv_lv.size()); ++type) {
        s_achv_lv[static_cast<size_t>(type)] = AchvData::get_level(type, AchvData::get_cur_amount(type));
    }
    while (!s_pending_unlocks.empty()) {
        s_pending_unlocks.pop_front();
    }
}

void AchvMng::check_achv_in_game(int type) {
    if (type < 0 || type >= static_cast<int>(s_achv_lv.size())) {
        return;
    }
    const int level = AchvData::get_level(type, AchvData::get_cur_amount(type));
    while (s_achv_lv[static_cast<size_t>(type)] < level && s_achv_lv[static_cast<size_t>(type)] < 3) {
        s_pending_unlocks.push_front({type, s_achv_lv[static_cast<size_t>(type)]});
        ++s_achv_lv[static_cast<size_t>(type)];
    }
}

void AchvMng::push_stage_score_in_game(int) {}

void AchvMng::push_battle_win_in_game(int) {}

void AchvMng::sign_in() {
    s_signed_in = true;
}

void AchvMng::sign_out() {
    s_signed_in = false;
}

bool AchvMng::is_sign_in() {
    return s_signed_in;
}

void AchvMng::show_achv_board() {}

void AchvMng::show_stage_board() {}

void AchvMng::push_achv_on_sign_in() {
    for (int type = 0; type < static_cast<int>(s_achv_lv.size()); ++type) {
        for (int level = 0; level < s_achv_lv[static_cast<size_t>(type)]; ++level) {
            s_pending_unlocks.push_front({type, level});
        }
    }
    if (Param::stage < 1) {
        Param::stage = Save::load_data(Save::STAGE);
    }
    if (Param::win < 1) {
        Param::win = Save::load_data(Save::WIN);
    }
    push_stage_score_in_game(Param::stage);
    push_battle_win_in_game(Param::win);
}

void AchvMng::update() {
    // Stub: online board push APIs are intentionally not implemented on Vita.
}

bool AchvMng::pop_unlocked(int& type, int& level) {
    if (s_pending_unlocks.empty()) {
        return false;
    }
    const UnlockEntry e = s_pending_unlocks.front();
    s_pending_unlocks.pop_front();
    type = e.type;
    level = e.level;
    return true;
}

int AchvMng::calc_amount_for_type(int type) {
    return AchvData::get_cur_amount(type);
}

int AchvMng::calc_level_for_type(int type, int amount) {
    return AchvData::get_level(type, amount);
}

} // namespace defender
