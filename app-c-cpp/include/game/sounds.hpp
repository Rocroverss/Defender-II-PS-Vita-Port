#pragma once

#include <array>
#include <string>

namespace defender {

struct SoundType {
    int resource_id = 0;
    bool stop_before_play = false;
    float volume = 1.0f;
    bool overlap_allowed = false;
};

class Sounds {
public:
    static constexpr const char* AUDIO_BASE_DIR = "assets/audio/";

    enum ResourceId {
        GAME_BGM = 1,
        BOSS,
        COVER_BGM,
        EXP_COUNT,
        BUTTON_CLICK,
        GAMEOVER_BGM,
        MANA_RECOVER,
        DH_LOGO,
        STAGECOMPLETE_BGM,
        BUTTON_UPGRADE,
        ARROW_SHOT,
        BEHIT,
        MONSTER_DEAD,
        DEVIL_FIREBALL_SHOT,
        DEVIL_FIREBALL_BLAST,
        FIRE_1,
        FIRE_2,
        ICE_1,
        ICE_2,
        LIGHTNING_1,
        MAGIC_READY,
        WALL_BEHIT,
        WALL_BROKEN,
        WARNING,
        LEVEL_UP,
        STONE_FIX,
        STONE_MOVE,
        STONE_THROW
    };

    static const SoundType STAGE_BGM;
    static const SoundType BOSS_BGM;
    static const SoundType COVER_BGM_SND;
    static const SoundType EXP_COUNT_SND;
    static const SoundType BUTTON_CLICK_SND;
    static const SoundType GAME_OVER;
    static const SoundType MANA_REC;
    static const SoundType DH_LOGO_SND;
    static const SoundType GAME_COMP;
    static const SoundType BUTTON_UPGREADE;
    static const SoundType ARROW_SHOT_SND;
    static const SoundType BEHIT_SND;
    static const SoundType DEAD;
    static const SoundType DEVIL_SHOT;
    static const SoundType DEVIL_FIREBALL_BLAST_SND;
    static const SoundType MONSTER_DEAD_SND;
    static const SoundType MAGIC_FIRE_1;
    static const SoundType MAGIC_FIRE_2;
    static const SoundType MAGIC_ICE_1;
    static const SoundType MAGIC_ICE_2;
    static const SoundType LIGHTNING_1_SND;
    static const SoundType MAGIC_READY_SND;
    static const SoundType WALL_BEHIT_SND;
    static const SoundType WALL_BROKEN_SND;
    static const SoundType WARNING_SND;
    static const SoundType LEVEL_UP_SND;
    static const SoundType STONE_FIX_SND;
    static const SoundType STONE_MOVE_SND;
    static const SoundType STONE_THROW_SND;

    static const std::array<SoundType, 29> ALL_SOUNDS;

    // Resolves Java-style sound resource ids to the Vita asset path under assets/audio.
    static std::string resource_file_name(int resource_id);
    static std::string resource_path(int resource_id);
    static std::string resource_path(const SoundType& sound);
    static int loop_cycle_duration_ms(int resource_id);
};

} // namespace defender
