#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace defender {

struct Param {
    static constexpr int DESIGNED_SCREEN_WIDTH = 800;
    static constexpr int DESIGNED_SCREEN_HEIGHT = 480;
    static constexpr int GAME_FRAME_LIMIT = 50;

    static constexpr int SCENE_COVER = 0;
    static constexpr int SCENE_MAIN_GAME = 1;
    static constexpr int SCENE_RESEARCH = 2;
    static constexpr int SCENE_STATS = 3;
    static constexpr int SCENE_LOADING = 4;
    static constexpr int SCENE_SHOP = 5;
    static constexpr int SCENE_ONLINE_DATA = 6;

    inline static int add_mana_data = 0;
    inline static int cast_fire = 0;
    inline static int cast_ice = 0;
    inline static int cast_light = 0;
    inline static int cost_coin = 0;
    inline static int cost_stone = 0;
    inline static int extra_battle_xp = 0;
    inline static int extra_dmg = 0;
    inline static int extra_fire = 0;
    inline static int extra_gold = 0;
    inline static int extra_ice = 0;
    inline static int extra_light = 0;
    inline static int extra_local_xp = 0;
    inline static int extra_mana = 0;
    inline static int fatal_blow_rate = 0;
    inline static int multi_power = 100;
    inline static int power_shot_dis = 0;
    inline static int atk_spd_dec_rate = 0;
    inline static int bt_level = 1;
    inline static int bt_time = 0;
    inline static int gold = 100;
    inline static int kills = 0;
    inline static int language = 0;
    inline static int level = 1;
    inline static int life_percent = 100;
    inline static int lose = 0;
    inline static int rep_time = 0;
    inline static int reward_stone = 0;
    inline static int single_battle_time = 0;
    inline static int stage = 1;
    inline static int stone = 2;
    inline static int time = 0;
    inline static int total_kills = 0;
    inline static int win = 0;
    inline static int xp = 0;

    inline static bool is_create = false;
    inline static bool is_online_mode = false;
    inline static bool is_show_ad = true;
    inline static bool is_win = false;
    inline static bool music_flag = true;
    inline static bool request_name_edit = false;
    inline static bool sound_effect_flag = true;

    inline static uint64_t random_seed1 = 1;
    inline static uint64_t random_seed2 = 2;
    inline static uint64_t random_seed3 = 3;

    inline static std::string player_name = "Defender";
    inline static std::array<int, 9> spell_data = {};
};

}
