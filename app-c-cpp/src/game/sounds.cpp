#include "game/sounds.hpp"

namespace defender {

namespace {

struct LoopTimingEntry {
    int resource_id;
    int cycle_duration_ms;
};

constexpr LoopTimingEntry kLoopTimingTable[] = {
    {Sounds::COVER_BGM, 9600},
    {Sounds::GAME_BGM, 48070},
    {Sounds::BOSS, 32000},
};

const char* file_name_for(int resource_id) {
    switch (resource_id) {
    case Sounds::GAME_BGM: return "game_bgm.ogg";
    case Sounds::BOSS: return "boss.ogg";
    case Sounds::COVER_BGM: return "cover_bgm.ogg";
    case Sounds::EXP_COUNT: return "exp_count.ogg";
    case Sounds::BUTTON_CLICK: return "button_click.ogg";
    case Sounds::GAMEOVER_BGM: return "gameover_bgm.wav";
    case Sounds::MANA_RECOVER: return "mana_recover.ogg";
    case Sounds::DH_LOGO: return "dh_logo.ogg";
    case Sounds::STAGECOMPLETE_BGM: return "stagecomplete_bgm.ogg";
    case Sounds::BUTTON_UPGRADE: return "button_upgrade.ogg";
    case Sounds::ARROW_SHOT: return "arrow_shot.wav";
    case Sounds::BEHIT: return "behit.wav";
    case Sounds::MONSTER_DEAD: return "monster_dead.ogg";
    case Sounds::DEVIL_FIREBALL_SHOT: return "devil_fireball_shot.wav";
    case Sounds::DEVIL_FIREBALL_BLAST: return "devil_fireball_blast.wav";
    case Sounds::FIRE_1: return "fire_1.ogg";
    case Sounds::FIRE_2: return "fire_2.ogg";
    case Sounds::ICE_1: return "ice_1.ogg";
    case Sounds::ICE_2: return "ice_2.ogg";
    case Sounds::LIGHTNING_1: return "lightning_1.ogg";
    case Sounds::MAGIC_READY: return "magic_ready.ogg";
    case Sounds::WALL_BEHIT: return "wall_behit.ogg";
    case Sounds::WALL_BROKEN: return "wall_broken.ogg";
    case Sounds::WARNING: return "warning.ogg";
    case Sounds::LEVEL_UP: return "level_up.ogg";
    case Sounds::STONE_FIX: return "stone_fix.ogg";
    case Sounds::STONE_MOVE: return "stone_move.ogg";
    case Sounds::STONE_THROW: return "stone_throw.ogg";
    default: return "";
    }
}

} // namespace

const SoundType Sounds::STAGE_BGM                 = {GAME_BGM, false, 1.0f, false};
const SoundType Sounds::BOSS_BGM                  = {BOSS, false, 1.0f, false};
const SoundType Sounds::COVER_BGM_SND             = {COVER_BGM, false, 1.0f, false};
const SoundType Sounds::EXP_COUNT_SND             = {EXP_COUNT, false, 0.5f, false};
const SoundType Sounds::BUTTON_CLICK_SND          = {BUTTON_CLICK, true, 1.0f, false};
const SoundType Sounds::GAME_OVER                 = {GAMEOVER_BGM, true, 1.0f, false};
const SoundType Sounds::MANA_REC                  = {MANA_RECOVER, true, 1.0f, false};
const SoundType Sounds::DH_LOGO_SND               = {DH_LOGO, true, 1.0f, false};
const SoundType Sounds::GAME_COMP                 = {STAGECOMPLETE_BGM, true, 1.0f, false};
const SoundType Sounds::BUTTON_UPGREADE           = {BUTTON_UPGRADE, true, 1.0f, false};
const SoundType Sounds::ARROW_SHOT_SND            = {ARROW_SHOT, true, 1.0f, false};
const SoundType Sounds::BEHIT_SND                 = {BEHIT, true, 1.0f, false};
const SoundType Sounds::DEAD                      = {MONSTER_DEAD, true, 1.0f, false};
const SoundType Sounds::DEVIL_SHOT                = {DEVIL_FIREBALL_SHOT, true, 1.0f, false};
const SoundType Sounds::DEVIL_FIREBALL_BLAST_SND  = {DEVIL_FIREBALL_BLAST, true, 1.0f, false};
const SoundType Sounds::MONSTER_DEAD_SND          = {MONSTER_DEAD, true, 1.0f, false};
const SoundType Sounds::MAGIC_FIRE_1              = {FIRE_1, true, 1.0f, false};
const SoundType Sounds::MAGIC_FIRE_2              = {FIRE_2, true, 1.0f, false};
const SoundType Sounds::MAGIC_ICE_1               = {ICE_1, true, 1.0f, false};
const SoundType Sounds::MAGIC_ICE_2               = {ICE_2, true, 1.0f, false};
const SoundType Sounds::LIGHTNING_1_SND           = {LIGHTNING_1, true, 1.0f, false};
const SoundType Sounds::MAGIC_READY_SND           = {MAGIC_READY, true, 1.0f, false};
const SoundType Sounds::WALL_BEHIT_SND            = {WALL_BEHIT, true, 1.0f, false};
const SoundType Sounds::WALL_BROKEN_SND           = {WALL_BROKEN, true, 1.0f, false};
const SoundType Sounds::WARNING_SND               = {WARNING, true, 1.0f, false};
const SoundType Sounds::LEVEL_UP_SND              = {LEVEL_UP, true, 1.0f, false};
const SoundType Sounds::STONE_FIX_SND             = {STONE_FIX, true, 1.0f, false};
const SoundType Sounds::STONE_MOVE_SND            = {STONE_MOVE, true, 1.0f, false};
const SoundType Sounds::STONE_THROW_SND           = {STONE_THROW, true, 1.0f, false};

const std::array<SoundType, 29> Sounds::ALL_SOUNDS = {{
    STAGE_BGM,
    BOSS_BGM,
    COVER_BGM_SND,
    EXP_COUNT_SND,
    BUTTON_CLICK_SND,
    GAME_OVER,
    GAME_COMP,
    BUTTON_UPGREADE,
    ARROW_SHOT_SND,
    BEHIT_SND,
    DEAD,
    DEVIL_SHOT,
    DEVIL_FIREBALL_BLAST_SND,
    MONSTER_DEAD_SND,
    MAGIC_FIRE_1,
    MAGIC_FIRE_2,
    MAGIC_ICE_1,
    MAGIC_ICE_2,
    LIGHTNING_1_SND,
    MAGIC_READY_SND,
    WALL_BEHIT_SND,
    WALL_BROKEN_SND,
    WARNING_SND,
    LEVEL_UP_SND,
    STONE_FIX_SND,
    STONE_MOVE_SND,
    STONE_THROW_SND,
    DH_LOGO_SND,
    MANA_REC
}};

std::string Sounds::resource_file_name(int resource_id) {
    return file_name_for(resource_id);
}

std::string Sounds::resource_path(int resource_id) {
    const char* file_name = file_name_for(resource_id);
    if (file_name == nullptr || file_name[0] == '\0') {
        return {};
    }
    return std::string(AUDIO_BASE_DIR) + file_name;
}

std::string Sounds::resource_path(const SoundType& sound) {
    return resource_path(sound.resource_id);
}

int Sounds::loop_cycle_duration_ms(int resource_id) {
    for (const auto& entry : kLoopTimingTable) {
        if (entry.resource_id == resource_id) {
            return entry.cycle_duration_ms;
        }
    }
    return 0;
}

} // namespace defender
