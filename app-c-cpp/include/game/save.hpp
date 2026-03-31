#pragma once

#include <cstdint>
#include <string>

namespace defender {

class Save {
public:
    static constexpr const char* BOW_GET = "bowGet";
    static constexpr const char* BOW_LOCK = "bowLock";
    static constexpr const char* BTL_LEVEL = "battleLevel";
    static constexpr const char* BTL_TIME = "battleTime";
    static constexpr const char* CHECK_DEVICEID = "checkDeviceID";
    static constexpr const char* CHECK_KEY = "checkChar";
    static constexpr const char* CHECK_VALUE = "checkValue";
    static constexpr const char* COST_COIN = "costCoin";
    static constexpr const char* COST_STONE = "costStone";
    static constexpr const char* DEVICEID = "deviceID";
    static constexpr const char* EQUIPED_BOW = "equipBow";
    static constexpr const char* EQUIPED_MAGIC = "equipMagic";
    static constexpr const char* FIRE_CAST = "fireCast";
    static constexpr int GLOBAL_DATA = 3;
    static constexpr const char* GOLD = "gold";
    static constexpr const char* HARD_MODE = "hardMode";
    static constexpr const char* HELP = "help";
    static constexpr const char* HIDE_AD = "isShowAd";
    static constexpr const char* ICE_CAST = "iceCast";
    static constexpr const char* KILLS = "killMonster";
    static constexpr const char* LEVEL = "level";
    static constexpr const char* LIGHT_CAST = "lightCast";
    static constexpr const char* LOSE = "loseGame";
    static constexpr const char* MUSIC_FLAG = "musicFlag";
    static constexpr const char* NAME = "playerName";
    static constexpr const char* NEWBIE_PACK = "newbiePack";
    static constexpr const char* PUR_RATE = "purchase";
    static constexpr int SAVE_INDEX_1 = 0;
    static constexpr int SAVE_INDEX_2 = 1;
    static constexpr int SAVE_INDEX_3 = 2;
    static constexpr const char* SHOW_DISCOUNT_PIC_TIME = "discountPicTime";
    static constexpr const char* SHOW_NEW_PACK_PIC_TIME = "newPackPicTime";
    static constexpr const char* SHOW_SUPER_PACK_PIC_TIME = "superPackPicTime";
    static constexpr const char* SHOW_TAPJOY_PIC_TIME = "tapjoyPicTime";
    static constexpr const char* SKILL_LEVEL = "skillLevel";
    static constexpr const char* SOUND_FLAG = "soundFlag";
    static constexpr const char* STAGE = "stage";
    static constexpr const char* STONE = "magicStone";
    static constexpr const char* SUPER_PACK = "superPack";
    static constexpr const char* TAPJOY_RATE = "tapjoy";
    static constexpr const char* WIN = "winGame";
    static constexpr const char* XP = "exp";

    static void init();

    static void save_data(const std::string& type, int value);
    static void save_data(const std::string& type, int value, int file_num);
    static void pause_save_data();

    static void save_time(const std::string& type, int64_t value);
    static int64_t load_time(const std::string& type);

    static void clear_data(int file_num);
    static void clear_data();

    static int load_data(const std::string& type);
    static int load_data(const std::string& type, int file_num);

    static std::string load_device_id();
    static std::string load_name();
    static void save_name(const std::string& name);

    static int get_default_value(const std::string& key);

private:
    static void ensure_default_values();
    static void new_game_init();
    static void add_value(const std::string& key, int value);
};

} // namespace defender

