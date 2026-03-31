#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <vitaGL.h>

namespace defender {

struct TextureHandle {
    GLuint id    = 0;
    int    width  = 0;
    int    height = 0;
    bool   valid  = false;
};

class TextureCache {
public:
    static TextureCache& instance();

    const TextureHandle& get(const std::string& relative_path);
    void clear();

private:
    TextureCache() = default;
    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;

    // BUG FIX: now returns the resolved path that actually succeeded so
    // callers can pass it on to the plist loader.
    TextureHandle load_texture_with_fallbacks(const std::string& relative_path,
                                              std::string& out_resolved_path) const;
    static TextureHandle load_texture_file(const std::string& full_path);

    std::unordered_map<std::string, TextureHandle> cache_;
};

} // namespace defender