#pragma once

#include "game/sounds.hpp"

namespace defender {

class AudioManager {
public:
    static AudioManager& instance();

    void initialize();
    void shutdown();
    void update();

    void play_sound(const SoundType& sound);
    void preload_sound(const SoundType& sound);
    void play_music(const SoundType& sound, bool loop = true, int loop_restart_delay_ms = 0);
    void stop_music();
    void stop_all();

    void set_music_enabled(bool enabled);
    void set_sound_enabled(bool enabled);
    void set_paused(bool paused);

    int current_music_resource() const;

private:
    AudioManager() = default;
    ~AudioManager() = default;
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
};

} // namespace defender
