#include "game/audio_manager.hpp"

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef __vita__
#include <psp2/audioout.h>
#if defined(__has_include)
#if __has_include(<vorbis/vorbisfile.h>)
#include <vorbis/vorbisfile.h>
#define DEFENDER_HAS_VORBIS 1
#else
#define DEFENDER_HAS_VORBIS 0
#endif
#else
#define DEFENDER_HAS_VORBIS 0
#endif
#endif

namespace defender {
namespace {

constexpr int kSampleRate = 48000;
constexpr size_t kMixFrames = 1024;
constexpr size_t kMaxVoices = 5;
constexpr uint64_t kMixBlockDurationMs = (kMixFrames * 1000ULL) / kSampleRate;
constexpr bool kAudioDebug = true;
constexpr const char* kDirectAppPathPrefix = "ux0:/app/DDEF00001/";

void audio_debug_log(const char* fmt, ...) {
    if (!kAudioDebug) {
        return;
    }
    std::va_list args;
    va_start(args, fmt);
    std::fputs("[audio] ", stdout);
    std::vfprintf(stdout, fmt, args);
    std::fputc('\n', stdout);
    std::fflush(stdout);
    va_end(args);
}

const char* resource_name(int resource_id) {
    switch (resource_id) {
    case Sounds::GAME_BGM: return "GAME_BGM";
    case Sounds::BOSS: return "BOSS";
    case Sounds::COVER_BGM: return "COVER_BGM";
    case Sounds::EXP_COUNT: return "EXP_COUNT";
    case Sounds::BUTTON_CLICK: return "BUTTON_CLICK";
    case Sounds::GAMEOVER_BGM: return "GAMEOVER_BGM";
    case Sounds::MANA_RECOVER: return "MANA_RECOVER";
    case Sounds::DH_LOGO: return "DH_LOGO";
    case Sounds::STAGECOMPLETE_BGM: return "STAGECOMPLETE_BGM";
    case Sounds::BUTTON_UPGRADE: return "BUTTON_UPGRADE";
    case Sounds::ARROW_SHOT: return "ARROW_SHOT";
    case Sounds::BEHIT: return "BEHIT";
    case Sounds::MONSTER_DEAD: return "MONSTER_DEAD";
    case Sounds::DEVIL_FIREBALL_SHOT: return "DEVIL_FIREBALL_SHOT";
    case Sounds::DEVIL_FIREBALL_BLAST: return "DEVIL_FIREBALL_BLAST";
    case Sounds::FIRE_1: return "FIRE_1";
    case Sounds::FIRE_2: return "FIRE_2";
    case Sounds::ICE_1: return "ICE_1";
    case Sounds::ICE_2: return "ICE_2";
    case Sounds::LIGHTNING_1: return "LIGHTNING_1";
    case Sounds::MAGIC_READY: return "MAGIC_READY";
    case Sounds::WALL_BEHIT: return "WALL_BEHIT";
    case Sounds::WALL_BROKEN: return "WALL_BROKEN";
    case Sounds::WARNING: return "WARNING";
    case Sounds::LEVEL_UP: return "LEVEL_UP";
    case Sounds::STONE_FIX: return "STONE_FIX";
    case Sounds::STONE_MOVE: return "STONE_MOVE";
    case Sounds::STONE_THROW: return "STONE_THROW";
    default: return "UNKNOWN";
    }
}

uint64_t frames_to_ms(size_t frames) {
    return (static_cast<uint64_t>(frames) * 1000ULL) / kSampleRate;
}

bool can_play_with_sound_disabled(int resource_id, bool music_enabled) {
    if (!music_enabled) {
        return false;
    }
    return resource_id == Sounds::GAMEOVER_BGM ||
           resource_id == Sounds::STAGECOMPLETE_BGM ||
           resource_id == Sounds::WARNING;
}

int sound_cooldown_ms(int resource_id) {
    switch (resource_id) {
    case Sounds::MAGIC_READY:
        return 50;
    case Sounds::ARROW_SHOT:
    case Sounds::BEHIT:
        return 35;
    case Sounds::DEVIL_FIREBALL_SHOT:
    case Sounds::DEVIL_FIREBALL_BLAST:
        return 45;
    default:
        return 0;
    }
}

struct SoundBuffer {
    int resource_id = 0;
    std::vector<int16_t> stereo_pcm;

    size_t frame_count() const {
        return stereo_pcm.size() / 2;
    }
};

struct Voice {
    std::shared_ptr<SoundBuffer> buffer;
    size_t frame_pos = 0;
    float volume = 1.0f;
    bool loop = false;
    bool allow_when_sound_disabled = false;
    int resource_id = 0;
    size_t loop_restart_delay_frames = 0;
    size_t silence_frames_remaining = 0;
};

#ifdef __vita__
class AudioBackend {
public:
    void initialize() {
        if (initialized_) {
            return;
        }
        port_ = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, static_cast<int>(kMixFrames), kSampleRate, SCE_AUDIO_OUT_MODE_STEREO);
        if (port_ < 0) {
            port_ = -1;
            return;
        }
        initialized_ = true;
        mix_accum_.assign(kMixFrames * 2, 0);
        mix_output_.assign(kMixFrames * 2, 0);
        last_sound_play_ms_.clear();
        mix_time_ms_ = 0;
    }

    void shutdown() {
        music_.reset();
        voices_.clear();
        cache_.clear();
        last_sound_play_ms_.clear();
        mix_time_ms_ = 0;
        audio_debug_log("shutdown");
        if (port_ >= 0) {
            sceAudioOutReleasePort(port_);
            port_ = -1;
        }
        initialized_ = false;
    }

    void update() {
        if (!initialized_ || port_ < 0) {
            return;
        }

        mix_time_ms_ += kMixBlockDurationMs;

        std::fill(mix_accum_.begin(), mix_accum_.end(), 0);

        if (!paused_) {
            if (music_enabled_ && music_) {
                if (!mix_voice(*music_, kMixFrames)) {
                    audio_debug_log(
                        "music ended name=%s id=%d played_ms=%llu",
                        resource_name(music_->resource_id),
                        music_->resource_id,
                        static_cast<unsigned long long>(frames_to_ms(music_->buffer ? music_->buffer->frame_count() : 0))
                    );
                    music_.reset();
                }
            }
            for (size_t i = 0; i < voices_.size();) {
                const bool can_mix_voice = sound_enabled_ || voices_[i].allow_when_sound_disabled;
                if (!can_mix_voice) {
                    audio_debug_log(
                        "voice dropped sound-disabled name=%s id=%d",
                        resource_name(voices_[i].resource_id),
                        voices_[i].resource_id
                    );
                    voices_.erase(voices_.begin() + static_cast<std::ptrdiff_t>(i));
                    continue;
                }
                if (!mix_voice(voices_[i], kMixFrames)) {
                    audio_debug_log(
                        "voice ended name=%s id=%d remaining_voices=%llu",
                        resource_name(voices_[i].resource_id),
                        voices_[i].resource_id,
                        static_cast<unsigned long long>(voices_.size() > 0 ? voices_.size() - 1 : 0)
                    );
                    voices_.erase(voices_.begin() + static_cast<std::ptrdiff_t>(i));
                } else {
                    ++i;
                }
            }
        }

        for (size_t i = 0; i < mix_accum_.size(); ++i) {
            mix_output_[i] = static_cast<int16_t>(std::clamp(mix_accum_[i], -32768, 32767));
        }
        sceAudioOutOutput(port_, mix_output_.data());
    }

    void play_sound(const SoundType& sound) {
        if (sound.resource_id == 0) {
            return;
        }
        audio_debug_log(
            "play_sound request name=%s id=%d stop_before=%d overlap=%d sound_enabled=%d music_enabled=%d active_voices=%llu",
            resource_name(sound.resource_id),
            sound.resource_id,
            sound.stop_before_play ? 1 : 0,
            sound.overlap_allowed ? 1 : 0,
            sound_enabled_ ? 1 : 0,
            music_enabled_ ? 1 : 0,
            static_cast<unsigned long long>(voices_.size())
        );
        if (!sound_enabled_ && !can_play_with_sound_disabled(sound.resource_id, music_enabled_)) {
            audio_debug_log("play_sound skipped disabled name=%s id=%d", resource_name(sound.resource_id), sound.resource_id);
            return;
        }
        const int cooldown_ms = sound_cooldown_ms(sound.resource_id);
        const auto found_play = last_sound_play_ms_.find(sound.resource_id);
        if (cooldown_ms > 0 &&
            found_play != last_sound_play_ms_.end() &&
            mix_time_ms_ < (found_play->second + static_cast<uint64_t>(cooldown_ms))) {
            audio_debug_log(
                "play_sound skipped cooldown name=%s id=%d cooldown_ms=%d elapsed_ms=%llu",
                resource_name(sound.resource_id),
                sound.resource_id,
                cooldown_ms,
                static_cast<unsigned long long>(mix_time_ms_ - found_play->second)
            );
            return;
        }
        auto buffer = load_sound(sound.resource_id);
        if (!buffer || buffer->stereo_pcm.empty()) {
            audio_debug_log("play_sound failed no-buffer name=%s id=%d", resource_name(sound.resource_id), sound.resource_id);
            return;
        }
        const uint64_t duration_ms = frames_to_ms(buffer->frame_count());
        size_t same_sound_active = 0;
        for (const auto& voice : voices_) {
            if (voice.resource_id == sound.resource_id) {
                ++same_sound_active;
            }
        }
        if (!sound.overlap_allowed && same_sound_active > 0) {
            audio_debug_log(
                "play_sound skipped already-active name=%s id=%d active_same=%llu duration_ms=%llu",
                resource_name(sound.resource_id),
                sound.resource_id,
                static_cast<unsigned long long>(same_sound_active),
                static_cast<unsigned long long>(duration_ms)
            );
            return;
        }

        size_t removed_same = 0;
        if (sound.stop_before_play || !sound.overlap_allowed) {
            const size_t before = voices_.size();
            voices_.erase(
                std::remove_if(
                    voices_.begin(),
                    voices_.end(),
                    [&sound](const Voice& voice) { return voice.resource_id == sound.resource_id; }
                ),
                voices_.end()
            );
            removed_same = before - voices_.size();
        }
        if (voices_.size() >= kMaxVoices) {
            audio_debug_log(
                "voice cap reached dropping_oldest dropped=%s id=%d",
                resource_name(voices_.front().resource_id),
                voices_.front().resource_id
            );
            voices_.erase(voices_.begin());
        }

        Voice voice;
        voice.buffer = std::move(buffer);
        voice.frame_pos = 0;
        voice.volume = sound.volume;
        voice.loop = false;
        voice.allow_when_sound_disabled = can_play_with_sound_disabled(sound.resource_id, music_enabled_);
        voice.resource_id = sound.resource_id;
        voice.loop_restart_delay_frames = 0;
        voice.silence_frames_remaining = 0;
        voices_.push_back(std::move(voice));
        last_sound_play_ms_[sound.resource_id] = mix_time_ms_;
        audio_debug_log(
            "play_sound started name=%s id=%d duration_ms=%llu removed_same=%llu active_now=%llu",
            resource_name(sound.resource_id),
            sound.resource_id,
            static_cast<unsigned long long>(duration_ms),
            static_cast<unsigned long long>(removed_same),
            static_cast<unsigned long long>(voices_.size())
        );
        if (voices_.size() > 1) {
            std::string active = "simultaneous voices:";
            for (const auto& item : voices_) {
                active += " ";
                active += resource_name(item.resource_id);
                active += "(" + std::to_string(item.resource_id) + ")";
            }
            audio_debug_log("%s", active.c_str());
        }
    }

    void preload_sound(const SoundType& sound) {
        if (sound.resource_id == 0) {
            return;
        }
        auto buffer = load_sound(sound.resource_id);
        if (!buffer || buffer->stereo_pcm.empty()) {
            audio_debug_log("preload_sound failed name=%s id=%d", resource_name(sound.resource_id), sound.resource_id);
            return;
        }
        audio_debug_log(
            "preload_sound ok name=%s id=%d duration_ms=%llu",
            resource_name(sound.resource_id),
            sound.resource_id,
            static_cast<unsigned long long>(frames_to_ms(buffer->frame_count()))
        );
    }

    void play_music(const SoundType& sound, bool loop, int loop_restart_delay_ms) {
        if (sound.resource_id == 0) {
            stop_music();
            return;
        }
        if (music_ && music_->resource_id == sound.resource_id) {
            music_->loop = loop;
            music_->volume = sound.volume;
            music_->loop_restart_delay_frames = delay_frames_for_ms(
                loop_restart_delay_ms,
                music_->buffer ? music_->buffer->frame_count() : 0
            );
            const uint64_t applied_restart_delay_ms = frames_to_ms(music_->loop_restart_delay_frames);
            audio_debug_log(
                "play_music reuse name=%s id=%d loop=%d restart_delay_ms=%llu duration_ms=%llu",
                resource_name(sound.resource_id),
                sound.resource_id,
                loop ? 1 : 0,
                static_cast<unsigned long long>(applied_restart_delay_ms),
                static_cast<unsigned long long>(frames_to_ms(music_->buffer ? music_->buffer->frame_count() : 0))
            );
            return;
        }

        auto buffer = load_sound(sound.resource_id);
        if (!buffer || buffer->stereo_pcm.empty()) {
            audio_debug_log("play_music failed no-buffer name=%s id=%d", resource_name(sound.resource_id), sound.resource_id);
            return;
        }
        const uint64_t duration_ms = frames_to_ms(buffer->frame_count());
        const size_t loop_restart_delay_frames = delay_frames_for_ms(loop_restart_delay_ms, buffer->frame_count());
        const uint64_t applied_restart_delay_ms = frames_to_ms(loop_restart_delay_frames);

        Voice music_voice;
        music_voice.buffer = std::move(buffer);
        music_voice.frame_pos = 0;
        music_voice.volume = sound.volume;
        music_voice.loop = loop;
        music_voice.allow_when_sound_disabled = false;
        music_voice.resource_id = sound.resource_id;
        music_voice.loop_restart_delay_frames = loop_restart_delay_frames;
        music_voice.silence_frames_remaining = 0;
        audio_debug_log(
            "play_music started name=%s id=%d loop=%d restart_delay_ms=%llu duration_ms=%llu",
            resource_name(sound.resource_id),
            sound.resource_id,
            loop ? 1 : 0,
            static_cast<unsigned long long>(applied_restart_delay_ms),
            static_cast<unsigned long long>(duration_ms)
        );
        music_ = std::make_unique<Voice>(std::move(music_voice));
    }

    void stop_music() {
        if (music_) {
            audio_debug_log("stop_music name=%s id=%d", resource_name(music_->resource_id), music_->resource_id);
        }
        music_.reset();
    }

    void stop_all() {
        audio_debug_log(
            "stop_all music=%d voices=%llu",
            music_ ? music_->resource_id : 0,
            static_cast<unsigned long long>(voices_.size())
        );
        music_.reset();
        voices_.clear();
    }

    void set_music_enabled(bool enabled) {
        if (music_enabled_ == enabled) {
            return;
        }
        music_enabled_ = enabled;
        audio_debug_log("set_music_enabled=%d", enabled ? 1 : 0);
    }

    void set_sound_enabled(bool enabled) {
        if (sound_enabled_ == enabled) {
            return;
        }
        sound_enabled_ = enabled;
        audio_debug_log("set_sound_enabled=%d", enabled ? 1 : 0);
        if (!enabled) {
            voices_.erase(
                std::remove_if(
                    voices_.begin(),
                    voices_.end(),
                    [](const Voice& voice) { return !voice.allow_when_sound_disabled; }
                ),
                voices_.end()
            );
        }
    }

    void set_paused(bool paused) {
        if (paused_ == paused) {
            return;
        }
        paused_ = paused;
        audio_debug_log("set_paused=%d", paused ? 1 : 0);
    }

    int current_music_resource() const {
        return music_ ? music_->resource_id : 0;
    }

private:
    static size_t delay_frames_for_ms(int cycle_duration_ms, size_t decoded_frames) {
        if (cycle_duration_ms <= 0) {
            return 0;
        }
        const uint64_t decoded_duration_ms = frames_to_ms(decoded_frames);
        if (static_cast<uint64_t>(cycle_duration_ms) <= decoded_duration_ms) {
            return 0;
        }
        const uint64_t silence_ms = static_cast<uint64_t>(cycle_duration_ms) - decoded_duration_ms;
        return static_cast<size_t>((silence_ms * kSampleRate) / 1000ULL);
    }

    static std::vector<std::string> candidate_paths_for(const std::string& relative_path) {
        std::vector<std::string> out;
        out.push_back(relative_path);
        out.push_back("app0:/" + relative_path);
        out.push_back(std::string(kDirectAppPathPrefix) + relative_path);
        out.push_back("../" + relative_path);
        return out;
    }

    static bool ends_with(const std::string& value, const char* suffix) {
        const size_t value_size = value.size();
        size_t suffix_size = 0;
        while (suffix[suffix_size] != '\0') {
            ++suffix_size;
        }
        if (value_size < suffix_size) {
            return false;
        }
        return value.compare(value_size - suffix_size, suffix_size, suffix) == 0;
    }

    static std::string alternate_audio_path(const std::string& relative_path) {
        if (ends_with(relative_path, ".wav")) {
            return relative_path.substr(0, relative_path.size() - 4) + ".ogg";
        }
        if (ends_with(relative_path, ".ogg")) {
            return relative_path.substr(0, relative_path.size() - 4) + ".wav";
        }
        return {};
    }

    static bool allow_alternate_format_fallback(int resource_id) {
        switch (resource_id) {
        case Sounds::GAME_BGM:
        case Sounds::BOSS:
        case Sounds::COVER_BGM:
            // The OGG decode path is currently not reliable for long looping BGM.
            // Keep menu/stage music pinned to the WAV masters so loop timing stays correct.
            return false;
        default:
            return true;
        }
    }

    static bool read_file_bytes(const std::string& relative_path, std::vector<uint8_t>& out_bytes) {
        for (const auto& candidate : candidate_paths_for(relative_path)) {
            FILE* fp = std::fopen(candidate.c_str(), "rb");
            if (fp == nullptr) {
                audio_debug_log("open failed candidate=%s", candidate.c_str());
                continue;
            }
            std::fseek(fp, 0, SEEK_END);
            const long size = std::ftell(fp);
            std::rewind(fp);
            if (size <= 0) {
                std::fclose(fp);
                audio_debug_log("open empty candidate=%s size=%ld", candidate.c_str(), size);
                continue;
            }
            out_bytes.resize(static_cast<size_t>(size));
            const size_t read = std::fread(out_bytes.data(), 1, out_bytes.size(), fp);
            std::fclose(fp);
            if (read == out_bytes.size()) {
                audio_debug_log(
                    "open ok candidate=%s size=%llu",
                    candidate.c_str(),
                    static_cast<unsigned long long>(out_bytes.size())
                );
                return true;
            }
            audio_debug_log(
                "open short-read candidate=%s expected=%llu read=%llu",
                candidate.c_str(),
                static_cast<unsigned long long>(out_bytes.size()),
                static_cast<unsigned long long>(read)
            );
            out_bytes.clear();
        }
        return false;
    }

    static int16_t read_s16_le(const uint8_t* ptr) {
        return static_cast<int16_t>(static_cast<uint16_t>(ptr[0]) | (static_cast<uint16_t>(ptr[1]) << 8));
    }

    static uint32_t read_u32_le(const uint8_t* ptr) {
        return static_cast<uint32_t>(ptr[0]) |
               (static_cast<uint32_t>(ptr[1]) << 8) |
               (static_cast<uint32_t>(ptr[2]) << 16) |
               (static_cast<uint32_t>(ptr[3]) << 24);
    }

    static std::vector<int16_t> resample_to_stereo(
        const std::vector<int16_t>& source_pcm,
        int src_channels,
        int src_rate
    ) {
        if (source_pcm.empty() || src_channels <= 0 || src_rate <= 0) {
            return {};
        }

        const size_t src_frames = source_pcm.size() / static_cast<size_t>(src_channels);
        if (src_frames == 0) {
            return {};
        }

        const uint64_t dst_frames_u64 =
            (static_cast<uint64_t>(src_frames) * static_cast<uint64_t>(kSampleRate)) /
            static_cast<uint64_t>(src_rate);
        const size_t dst_frames = static_cast<size_t>(std::max<uint64_t>(1ULL, dst_frames_u64));
        std::vector<int16_t> out(dst_frames * 2, 0);

        for (size_t i = 0; i < dst_frames; ++i) {
            const double src_pos = (static_cast<double>(i) * static_cast<double>(src_rate)) / static_cast<double>(kSampleRate);
            const size_t idx0 = std::min(src_frames - 1, static_cast<size_t>(src_pos));
            const size_t idx1 = std::min(src_frames - 1, idx0 + 1);
            const double frac = src_pos - static_cast<double>(idx0);

            for (int ch = 0; ch < 2; ++ch) {
                const int src_ch = std::min(ch, src_channels - 1);
                const int16_t s0 = source_pcm[(idx0 * static_cast<size_t>(src_channels)) + static_cast<size_t>(src_ch)];
                const int16_t s1 = source_pcm[(idx1 * static_cast<size_t>(src_channels)) + static_cast<size_t>(src_ch)];
                const double mixed = (static_cast<double>(s0) * (1.0 - frac)) + (static_cast<double>(s1) * frac);
                out[(i * 2) + static_cast<size_t>(ch)] = static_cast<int16_t>(std::clamp(static_cast<int>(mixed), -32768, 32767));
            }
        }

        return out;
    }

    static std::shared_ptr<SoundBuffer> decode_wav(const std::string& relative_path, int resource_id) {
        std::vector<uint8_t> bytes;
        if (!read_file_bytes(relative_path, bytes) || bytes.size() < 44) {
            audio_debug_log(
                "decode_wav failed read name=%s id=%d bytes=%llu",
                resource_name(resource_id),
                resource_id,
                static_cast<unsigned long long>(bytes.size())
            );
            return {};
        }
        if (std::memcmp(bytes.data(), "RIFF", 4) != 0 || std::memcmp(bytes.data() + 8, "WAVE", 4) != 0) {
            audio_debug_log("decode_wav failed header name=%s id=%d", resource_name(resource_id), resource_id);
            return {};
        }

        uint16_t audio_format = 0;
        uint16_t channels = 0;
        uint32_t sample_rate = 0;
        uint16_t bits_per_sample = 0;
        const uint8_t* data_ptr = nullptr;
        size_t data_size = 0;

        size_t offset = 12;
        while (offset + 8 <= bytes.size()) {
            const uint8_t* chunk = bytes.data() + offset;
            const uint32_t chunk_size = read_u32_le(chunk + 4);
            const size_t chunk_data_offset = offset + 8;
            if (chunk_data_offset + chunk_size > bytes.size()) {
                break;
            }

            if (std::memcmp(chunk, "fmt ", 4) == 0 && chunk_size >= 16) {
                audio_format = static_cast<uint16_t>(chunk[8] | (chunk[9] << 8));
                channels = static_cast<uint16_t>(chunk[10] | (chunk[11] << 8));
                sample_rate = read_u32_le(chunk + 12);
                bits_per_sample = static_cast<uint16_t>(chunk[22] | (chunk[23] << 8));
            } else if (std::memcmp(chunk, "data", 4) == 0) {
                data_ptr = bytes.data() + chunk_data_offset;
                data_size = chunk_size;
            }

            offset = chunk_data_offset + chunk_size + (chunk_size & 1U);
        }

        if (audio_format != 1 || (channels != 1 && channels != 2) || sample_rate == 0 || data_ptr == nullptr || data_size == 0) {
            audio_debug_log(
                "decode_wav failed format name=%s id=%d format=%u channels=%u rate=%u bits=%u data=%llu",
                resource_name(resource_id),
                resource_id,
                static_cast<unsigned>(audio_format),
                static_cast<unsigned>(channels),
                static_cast<unsigned>(sample_rate),
                static_cast<unsigned>(bits_per_sample),
                static_cast<unsigned long long>(data_size)
            );
            return {};
        }

        std::vector<int16_t> decoded_pcm;
        if (bits_per_sample == 8) {
            decoded_pcm.resize(data_size);
            for (size_t i = 0; i < data_size; ++i) {
                decoded_pcm[i] = static_cast<int16_t>((static_cast<int>(data_ptr[i]) - 128) << 8);
            }
        } else if (bits_per_sample == 16) {
            decoded_pcm.resize(data_size / 2);
            for (size_t i = 0; i < decoded_pcm.size(); ++i) {
                decoded_pcm[i] = read_s16_le(data_ptr + (i * 2));
            }
        } else {
            audio_debug_log(
                "decode_wav failed bits name=%s id=%d bits=%u",
                resource_name(resource_id),
                resource_id,
                static_cast<unsigned>(bits_per_sample)
            );
            return {};
        }

        auto sound = std::make_shared<SoundBuffer>();
        sound->resource_id = resource_id;
        sound->stereo_pcm = resample_to_stereo(decoded_pcm, channels, static_cast<int>(sample_rate));
        return sound;
    }

#if DEFENDER_HAS_VORBIS
    struct MemoryOggSource {
        const uint8_t* data = nullptr;
        size_t size = 0;
        size_t offset = 0;
    };

    static size_t ogg_read_callback(void* ptr, size_t size, size_t nmemb, void* datasource) {
        if (ptr == nullptr || datasource == nullptr || size == 0 || nmemb == 0) {
            return 0;
        }
        auto* source = static_cast<MemoryOggSource*>(datasource);
        const size_t bytes_requested = size * nmemb;
        const size_t bytes_remaining = (source->offset < source->size) ? (source->size - source->offset) : 0;
        const size_t bytes_to_copy = std::min(bytes_requested, bytes_remaining);
        if (bytes_to_copy == 0) {
            return 0;
        }
        std::memcpy(ptr, source->data + source->offset, bytes_to_copy);
        source->offset += bytes_to_copy;
        return bytes_to_copy / size;
    }

    static int ogg_seek_callback(void* datasource, ogg_int64_t offset, int whence) {
        if (datasource == nullptr) {
            return -1;
        }
        auto* source = static_cast<MemoryOggSource*>(datasource);
        size_t new_offset = 0;
        switch (whence) {
        case SEEK_SET:
            if (offset < 0) {
                return -1;
            }
            new_offset = static_cast<size_t>(offset);
            break;
        case SEEK_CUR:
            if (offset < 0 && static_cast<size_t>(-offset) > source->offset) {
                return -1;
            }
            new_offset = (offset < 0)
                ? (source->offset - static_cast<size_t>(-offset))
                : (source->offset + static_cast<size_t>(offset));
            break;
        case SEEK_END:
            if (offset < 0 && static_cast<size_t>(-offset) > source->size) {
                return -1;
            }
            new_offset = (offset < 0)
                ? (source->size - static_cast<size_t>(-offset))
                : (source->size + static_cast<size_t>(offset));
            break;
        default:
            return -1;
        }
        if (new_offset > source->size) {
            return -1;
        }
        source->offset = new_offset;
        return 0;
    }

    static int ogg_close_callback(void* datasource) {
        (void)datasource;
        return 0;
    }

    static long ogg_tell_callback(void* datasource) {
        if (datasource == nullptr) {
            return -1;
        }
        auto* source = static_cast<MemoryOggSource*>(datasource);
        return static_cast<long>(source->offset);
    }

    static std::shared_ptr<SoundBuffer> decode_ogg(const std::string& relative_path, int resource_id) {
        std::vector<uint8_t> bytes;
        if (!read_file_bytes(relative_path, bytes) || bytes.empty()) {
            audio_debug_log("decode_ogg failed read name=%s id=%d", resource_name(resource_id), resource_id);
            return {};
        }

        MemoryOggSource source;
        source.data = bytes.data();
        source.size = bytes.size();
        source.offset = 0;

        ov_callbacks callbacks{};
        callbacks.read_func = ogg_read_callback;
        callbacks.seek_func = ogg_seek_callback;
        callbacks.close_func = ogg_close_callback;
        callbacks.tell_func = ogg_tell_callback;

        OggVorbis_File vf{};
        const int open_result = ov_open_callbacks(&source, &vf, nullptr, 0, callbacks);
        if (open_result != 0) {
            audio_debug_log(
                "decode_ogg failed open name=%s id=%d error=%d bytes=%llu",
                resource_name(resource_id),
                resource_id,
                open_result,
                static_cast<unsigned long long>(bytes.size())
            );
            return {};
        }

        vorbis_info* info = ov_info(&vf, -1);
        if (info == nullptr || info->channels <= 0 || info->rate <= 0) {
            audio_debug_log("decode_ogg failed info name=%s id=%d", resource_name(resource_id), resource_id);
            ov_clear(&vf);
            return {};
        }
        const int channels = info->channels;
        const int sample_rate = info->rate;
        const ogg_int64_t total_pcm_frames = ov_pcm_total(&vf, -1);
        const double total_time_s = ov_time_total(&vf, -1);

        std::vector<int16_t> decoded_pcm;
        if (total_pcm_frames > 0) {
            decoded_pcm.reserve(static_cast<size_t>(total_pcm_frames) * static_cast<size_t>(channels));
        }

        std::array<char, 16384> chunk{};
        int bitstream = 0;
        size_t total_bytes_read = 0;
        while (true) {
            const long read = ov_read(&vf, chunk.data(), static_cast<int>(chunk.size()), 0, 2, 1, &bitstream);
            if (read == 0) {
                break;
            }
            if (read < 0) {
                if (read == OV_HOLE) {
                    continue;
                }
                audio_debug_log(
                    "decode_ogg failed stream name=%s id=%d error=%ld bytes=%llu",
                    resource_name(resource_id),
                    resource_id,
                    read,
                    static_cast<unsigned long long>(total_bytes_read)
                );
                decoded_pcm.clear();
                break;
            }
            const size_t sample_count = static_cast<size_t>(read) / sizeof(int16_t);
            const size_t old_size = decoded_pcm.size();
            decoded_pcm.resize(old_size + sample_count);
            std::memcpy(decoded_pcm.data() + old_size, chunk.data(), static_cast<size_t>(read));
            total_bytes_read += static_cast<size_t>(read);
        }
        ov_clear(&vf);

        if (decoded_pcm.empty()) {
            audio_debug_log("decode_ogg failed empty name=%s id=%d", resource_name(resource_id), resource_id);
            return {};
        }

        audio_debug_log(
            "decode_ogg ok name=%s id=%d file_bytes=%llu pcm_frames=%lld time_ms=%llu read_bytes=%llu channels=%d rate=%d",
            resource_name(resource_id),
            resource_id,
            static_cast<unsigned long long>(bytes.size()),
            static_cast<long long>(total_pcm_frames),
            static_cast<unsigned long long>(total_time_s > 0.0 ? (total_time_s * 1000.0) : 0.0),
            static_cast<unsigned long long>(total_bytes_read),
            channels,
            sample_rate
        );

        auto sound = std::make_shared<SoundBuffer>();
        sound->resource_id = resource_id;
        sound->stereo_pcm = resample_to_stereo(decoded_pcm, channels, sample_rate);
        return sound;
    }
#endif

    std::shared_ptr<SoundBuffer> decode_by_path(const std::string& path, int resource_id) {
        if (path.empty()) {
            return {};
        }

        if (ends_with(path, ".wav")) {
            return decode_wav(path, resource_id);
        }
#if DEFENDER_HAS_VORBIS
        if (ends_with(path, ".ogg")) {
            return decode_ogg(path, resource_id);
        }
#endif
        return {};
    }

    std::shared_ptr<SoundBuffer> load_sound(int resource_id) {
        const auto found = cache_.find(resource_id);
        if (found != cache_.end()) {
            return found->second;
        }

        const std::string primary_path = Sounds::resource_path(resource_id);
        if (primary_path.empty()) {
            return {};
        }

        std::shared_ptr<SoundBuffer> sound = decode_by_path(primary_path, resource_id);
        bool used_fallback = false;
        std::string loaded_path = primary_path;
        const std::string fallback_path = alternate_audio_path(primary_path);
        if (!sound && allow_alternate_format_fallback(resource_id)) {
            if (!fallback_path.empty()) {
                sound = decode_by_path(fallback_path, resource_id);
                used_fallback = sound != nullptr;
                if (used_fallback) {
                    loaded_path = fallback_path;
                }
            }
        }

        if (sound) {
            cache_[resource_id] = sound;
            audio_debug_log(
                "loaded name=%s id=%d path=%s fallback=%d duration_ms=%llu frames=%llu",
                resource_name(resource_id),
                resource_id,
                loaded_path.c_str(),
                used_fallback ? 1 : 0,
                static_cast<unsigned long long>(frames_to_ms(sound->frame_count())),
                static_cast<unsigned long long>(sound->frame_count())
            );
        } else {
            audio_debug_log(
                "load failed name=%s id=%d primary=%s fallback=%s fallback_allowed=%d",
                resource_name(resource_id),
                resource_id,
                primary_path.c_str(),
                fallback_path.c_str(),
                allow_alternate_format_fallback(resource_id) ? 1 : 0
            );
        }
        return sound;
    }

    bool mix_voice(Voice& voice, size_t frame_count) {
        if (!voice.buffer || voice.buffer->stereo_pcm.empty()) {
            return false;
        }

        const size_t total_frames = voice.buffer->frame_count();
        if (total_frames == 0) {
            return false;
        }

        size_t mixed_frames = 0;
        while (mixed_frames < frame_count) {
            if (voice.silence_frames_remaining > 0) {
                const size_t silent_frames = std::min(frame_count - mixed_frames, voice.silence_frames_remaining);
                voice.silence_frames_remaining -= silent_frames;
                mixed_frames += silent_frames;
                continue;
            }
            if (voice.frame_pos >= total_frames) {
                if (!voice.loop) {
                    return false;
                }
                voice.frame_pos = 0;
                if (voice.loop_restart_delay_frames > 0) {
                    audio_debug_log(
                        "loop delay name=%s id=%d delay_ms=%llu",
                        resource_name(voice.resource_id),
                        voice.resource_id,
                        static_cast<unsigned long long>(frames_to_ms(voice.loop_restart_delay_frames))
                    );
                    voice.silence_frames_remaining = voice.loop_restart_delay_frames;
                    continue;
                }
                audio_debug_log("loop restart immediate name=%s id=%d", resource_name(voice.resource_id), voice.resource_id);
            }

            const size_t available_frames = total_frames - voice.frame_pos;
            const size_t span_frames = std::min(frame_count - mixed_frames, available_frames);
            const size_t src_index = voice.frame_pos * 2;
            const size_t dst_index = mixed_frames * 2;

            for (size_t i = 0; i < span_frames; ++i) {
                const size_t src_sample = src_index + (i * 2);
                const size_t dst_sample = dst_index + (i * 2);
                mix_accum_[dst_sample] += static_cast<int32_t>(static_cast<float>(voice.buffer->stereo_pcm[src_sample]) * voice.volume);
                mix_accum_[dst_sample + 1] += static_cast<int32_t>(static_cast<float>(voice.buffer->stereo_pcm[src_sample + 1]) * voice.volume);
            }

            voice.frame_pos += span_frames;
            mixed_frames += span_frames;
        }

        return true;
    }

    bool initialized_ = false;
    bool music_enabled_ = true;
    bool sound_enabled_ = true;
    bool paused_ = false;
    int port_ = -1;
    std::unordered_map<int, std::shared_ptr<SoundBuffer>> cache_;
    std::unordered_map<int, uint64_t> last_sound_play_ms_;
    std::vector<int32_t> mix_accum_;
    std::vector<int16_t> mix_output_;
    std::unique_ptr<Voice> music_;
    std::vector<Voice> voices_;
    uint64_t mix_time_ms_ = 0;
};
#else
class AudioBackend {
public:
    void initialize() {}
    void shutdown() {}
    void update() {}
    void play_sound(const SoundType&) {}
    void preload_sound(const SoundType&) {}
    void play_music(const SoundType&, bool, int) {}
    void stop_music() {}
    void stop_all() {}
    void set_music_enabled(bool) {}
    void set_sound_enabled(bool) {}
    void set_paused(bool) {}
    int current_music_resource() const { return 0; }
};
#endif

AudioBackend& backend() {
    static AudioBackend s_backend;
    return s_backend;
}

} // namespace

AudioManager& AudioManager::instance() {
    static AudioManager s_manager;
    return s_manager;
}

void AudioManager::initialize() {
    backend().initialize();
}

void AudioManager::shutdown() {
    backend().shutdown();
}

void AudioManager::update() {
    backend().update();
}

void AudioManager::play_sound(const SoundType& sound) {
    backend().play_sound(sound);
}

void AudioManager::preload_sound(const SoundType& sound) {
    backend().preload_sound(sound);
}

void AudioManager::play_music(const SoundType& sound, bool loop, int loop_restart_delay_ms) {
    backend().play_music(sound, loop, loop_restart_delay_ms);
}

void AudioManager::stop_music() {
    backend().stop_music();
}

void AudioManager::stop_all() {
    backend().stop_all();
}

void AudioManager::set_music_enabled(bool enabled) {
    backend().set_music_enabled(enabled);
}

void AudioManager::set_sound_enabled(bool enabled) {
    backend().set_sound_enabled(enabled);
}

void AudioManager::set_paused(bool paused) {
    backend().set_paused(paused);
}

int AudioManager::current_music_resource() const {
    return backend().current_music_resource();
}

} // namespace defender
