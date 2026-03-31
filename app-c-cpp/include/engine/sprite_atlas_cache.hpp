#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "engine/texture_cache.hpp"

namespace defender {

struct AtlasFrame {
    GLuint texture_id = 0;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
    int width = 0;
    int height = 0;
    int original_width = 0;
    int original_height = 0;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    bool valid = false;
};

class SpriteAtlasCache {
public:
    static SpriteAtlasCache& instance();

    const AtlasFrame* get_frame(const std::string& frame_name);
    void ensure_companion_plist_loaded(const std::string& texture_relative_path);
    void warmup_default();
    void clear();

private:
    SpriteAtlasCache() = default;
    SpriteAtlasCache(const SpriteAtlasCache&) = delete;
    SpriteAtlasCache& operator=(const SpriteAtlasCache&) = delete;

    bool load_plist(const std::string& plist_relative_path);

    std::unordered_map<std::string, AtlasFrame> frames_;
    std::unordered_set<std::string> loaded_plists_;
    bool warmed_ = false;
};

} // namespace defender
