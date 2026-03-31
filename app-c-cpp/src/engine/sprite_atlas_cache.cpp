#include "engine/sprite_atlas_cache.hpp"

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

// NOTE: <fstream> / std::ifstream is intentionally NOT used.
// On PS Vita, std::ifstream does NOT honour the "app0:/" URI scheme that
// vitaSDK maps to the installed application's read-only data partition.
// Only the C stdio functions (fopen / fread / fclose) are routed through
// the Vita's sceIo layer and therefore work correctly with "app0:/".

namespace defender {

namespace {

constexpr bool kPlistDebugLog = true;

void plist_log(const char* fmt, ...) {
    if (!kPlistDebugLog) return;
    std::va_list args;
    va_start(args, fmt);
    std::printf("[plist] ");
    std::vprintf(fmt, args);
    std::printf("\n");
    va_end(args);
}

std::string trim_copy(const std::string& s) {
    size_t begin = 0;
    while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin])) != 0)
        ++begin;
    size_t end = s.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1])) != 0)
        --end;
    return s.substr(begin, end - begin);
}

std::string to_lower_copy(std::string s) {
    for (char& c : s)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool is_always_plist_texture_path_lower(const std::string& lower_path) {
    std::string normalized = lower_path;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    if (normalized.rfind("app0:/", 0) == 0) {
        normalized.erase(0, 6);
    }
    while (normalized.rfind("../", 0) == 0) {
        normalized.erase(0, 3);
    }
    while (normalized.rfind("./", 0) == 0) {
        normalized.erase(0, 2);
    }
    return normalized.rfind("assets/imgs_480_800/always/plist/", 0) == 0;
}

bool extract_key_tag(const std::string& line, std::string& out) {
    const std::string open  = "<key>";
    const std::string close = "</key>";
    const size_t p0 = line.find(open);
    const size_t p1 = line.find(close);
    if (p0 == std::string::npos || p1 == std::string::npos || p1 <= p0 + open.size())
        return false;
    out = line.substr(p0 + open.size(), p1 - (p0 + open.size()));
    return true;
}

bool extract_number_tag(const std::string& line, float& out) {
    auto parse_between = [&line, &out](const char* open, const char* close) -> bool {
        const size_t p0 = line.find(open);
        const size_t p1 = line.find(close);
        if (p0 == std::string::npos || p1 == std::string::npos ||
            p1 <= p0 + std::strlen(open))
            return false;
        const std::string text = line.substr(p0 + std::strlen(open),
                                             p1 - (p0 + std::strlen(open)));
        out = static_cast<float>(std::strtod(text.c_str(), nullptr));
        return true;
    };
    return parse_between("<integer>", "</integer>") ||
           parse_between("<real>",    "</real>");
}

std::string plist_to_png_path(const std::string& plist_path) {
    const size_t dot = plist_path.find_last_of('.');
    if (dot == std::string::npos) return plist_path + ".png";
    return plist_path.substr(0, dot) + ".png";
}

std::string plist_from_texture_path(const std::string& texture_path) {
    const size_t dot = texture_path.find_last_of('.');
    if (dot == std::string::npos) return texture_path + ".plist";
    return texture_path.substr(0, dot) + ".plist";
}

// -----------------------------------------------------------------------
// BUG FIX: replaced std::ifstream with C stdio fopen/fread.
//
// std::ifstream on PS Vita does NOT support the "app0:/" URI prefix used
// by vitaSDK — it tries to open the path as a raw POSIX string and fails.
// fopen() goes through sceIoOpen under the hood and correctly resolves
// "app0:/", "ux0:", etc.
//
// The candidate list is built the same way as in texture_cache.cpp so the
// two subsystems are always in agreement about which paths to try.
// -----------------------------------------------------------------------
bool read_lines_with_fallbacks(const std::string& relative_path,
                               std::vector<std::string>& out_lines) {
    plist_log("lookup companion plist: %s", relative_path.c_str());

    // Build candidates — identical logic to plist_candidates_for() in
    // texture_cache.cpp so the two subsystems always agree.
    std::vector<std::string> candidates;
    candidates.push_back(relative_path);
    if (relative_path.rfind("app0:/", 0) != 0) {
        candidates.push_back("app0:/" + relative_path);
    }
    candidates.push_back("../" + relative_path);

    for (const auto& path : candidates) {
        // Use fopen, NOT std::ifstream — the only reliable way on Vita.
        FILE* fp = std::fopen(path.c_str(), "rb");
        if (fp == nullptr) {
            plist_log("not found at candidate: %s", path.c_str());
            continue;
        }

        // Read the entire file then split into lines manually.
        std::fseek(fp, 0, SEEK_END);
        const long file_size = std::ftell(fp);
        std::fseek(fp, 0, SEEK_SET);

        if (file_size <= 0) {
            std::fclose(fp);
            plist_log("empty file at candidate: %s", path.c_str());
            continue;
        }

        std::string contents(static_cast<size_t>(file_size), '\0');
        const size_t bytes_read = std::fread(&contents[0], 1,
                                             static_cast<size_t>(file_size), fp);
        std::fclose(fp);

        if (bytes_read == 0) {
            plist_log("fread returned 0 for: %s", path.c_str());
            continue;
        }
        contents.resize(bytes_read); // trim if short read

        // Split into lines (handles \r\n and \n).
        out_lines.clear();
        size_t pos = 0;
        while (pos < contents.size()) {
            const size_t newline = contents.find('\n', pos);
            size_t end = (newline == std::string::npos) ? contents.size() : newline;
            // Strip trailing \r
            if (end > pos && contents[end - 1] == '\r') --end;
            out_lines.push_back(contents.substr(pos, end - pos));
            pos = (newline == std::string::npos) ? contents.size() : newline + 1;
        }

        plist_log("opened plist ok: %s (lines=%d)",
                  path.c_str(), static_cast<int>(out_lines.size()));
        return true;
    }

    plist_log("plist open failed for all candidates: %s", relative_path.c_str());
    return false;
}

} // namespace

// ---------------------------------------------------------------------------
// SpriteAtlasCache
// ---------------------------------------------------------------------------

SpriteAtlasCache& SpriteAtlasCache::instance() {
    static SpriteAtlasCache cache;
    return cache;
}

const AtlasFrame* SpriteAtlasCache::get_frame(const std::string& frame_name) {
    if (!warmed_) warmup_default();
    const auto it = frames_.find(frame_name);
    if (it == frames_.end()) return nullptr;
    return &it->second;
}

void SpriteAtlasCache::ensure_companion_plist_loaded(const std::string& texture_resolved_path) {
    // texture_resolved_path is the path that fopen actually succeeded on
    // (e.g. "app0:/assets/.../bow_normal.png"), NOT necessarily the bare
    // relative path — this is the key fix that makes the plist loader try
    // the same prefix the texture loader already confirmed works.
    const std::string lower = to_lower_copy(texture_resolved_path);
    if (lower.size() < 4 || lower.substr(lower.size() - 4) != ".png") return;
    if (!is_always_plist_texture_path_lower(lower)) return;

    const std::string plist_path = plist_from_texture_path(texture_resolved_path);
    const bool ok = load_plist(plist_path);
    if (ok) {
        plist_log("companion load success: %s <- %s",
                  plist_path.c_str(), texture_resolved_path.c_str());
    } else {
        plist_log("companion load failed: %s <- %s",
                  plist_path.c_str(), texture_resolved_path.c_str());
    }
}

void SpriteAtlasCache::warmup_default() {
    if (warmed_) return;

    static const char* kDefaultPlists[] = {
        "assets/imgs_480_800/always/plist/monster1.plist",
        "assets/imgs_480_800/always/plist/monster2.plist",
        "assets/imgs_480_800/always/plist/monster3.plist",
        "assets/imgs_480_800/always/plist/monster4.plist",
        "assets/imgs_480_800/always/plist/monster5.plist",
        "assets/imgs_480_800/always/plist/monster6.plist",
        "assets/imgs_480_800/always/plist/monster7.plist",
        "assets/imgs_480_800/always/plist/monster8.plist",
        "assets/imgs_480_800/always/plist/magic.plist",
        "assets/imgs_480_800/always/plist/magic2.plist",
        "assets/imgs_480_800/always/plist/magic3.plist",
        "assets/imgs_480_800/always/plist/magic4.plist",
        "assets/imgs_480_800/always/plist/research1.plist",
        "assets/imgs_480_800/always/plist/research2.plist",
        "assets/imgs_480_800/always/plist/achv_button.plist",
    };

    for (const char* plist : kDefaultPlists) {
        load_plist(plist);
    }

    warmed_ = true;
}

void SpriteAtlasCache::clear() {
    frames_.clear();
    loaded_plists_.clear();
    warmed_ = false;
}

bool SpriteAtlasCache::load_plist(const std::string& plist_relative_path) {
    const auto [_, inserted] = loaded_plists_.insert(plist_relative_path);
    if (!inserted) {
        plist_log("already loaded, skip: %s", plist_relative_path.c_str());
        return true;
    }

    std::vector<std::string> lines;
    if (!read_lines_with_fallbacks(plist_relative_path, lines)) {
        loaded_plists_.erase(plist_relative_path);
        plist_log("read failed: %s", plist_relative_path.c_str());
        return false;
    }

    const std::string texture_path = plist_to_png_path(plist_relative_path);
    const TextureHandle& atlas_tex = TextureCache::instance().get(texture_path);
    if (!atlas_tex.valid || atlas_tex.id == 0) {
        loaded_plists_.erase(plist_relative_path);
        plist_log("atlas texture missing/invalid for plist: %s (texture=%s)",
                  plist_relative_path.c_str(), texture_path.c_str());
        return false;
    }
    plist_log("atlas texture ready: %s (id=%u, w=%d, h=%d)",
              texture_path.c_str(),
              static_cast<unsigned int>(atlas_tex.id),
              atlas_tex.width, atlas_tex.height);

    struct TempFrame {
        int   x = 0, y = 0;
        int   width = 0, height = 0;
        int   original_width = 0, original_height = 0;
        float offset_x = 0.0f, offset_y = 0.0f;
    };

    int texture_width  = 0;
    int texture_height = 0;

    bool pending_texture_dict = false;
    bool pending_frames_dict  = false;
    bool in_texture_dict      = false;
    bool in_frames_dict       = false;
    bool in_frame_dict        = false;

    std::string pending_frame_name;
    std::string pending_field;
    TempFrame   temp_frame{};
    int frame_insert_count    = 0;
    int frame_duplicate_count = 0;

    auto flush_frame = [&]() {
        if (pending_frame_name.empty() ||
            temp_frame.width  <= 0    ||
            temp_frame.height <= 0)
            return;

        if (frames_.find(pending_frame_name) != frames_.end()) {
            ++frame_duplicate_count;
            return;
        }

        const int atlas_w = texture_width  > 0 ? texture_width  : atlas_tex.width;
        const int atlas_h = texture_height > 0 ? texture_height : atlas_tex.height;
        if (atlas_w <= 0 || atlas_h <= 0) return;

        AtlasFrame frame;
        frame.texture_id      = atlas_tex.id;
        frame.width           = temp_frame.width;
        frame.height          = temp_frame.height;
        frame.original_width  = temp_frame.original_width  > 0 ? temp_frame.original_width  : temp_frame.width;
        frame.original_height = temp_frame.original_height > 0 ? temp_frame.original_height : temp_frame.height;
        frame.offset_x        = temp_frame.offset_x;
        frame.offset_y        = temp_frame.offset_y;
        frame.u0 = static_cast<float>(temp_frame.x) / static_cast<float>(atlas_w);
        frame.u1 = static_cast<float>(temp_frame.x + temp_frame.width) / static_cast<float>(atlas_w);
        // Java parity (PlistTexture + BoundUtil): plist y is top-origin.
        // draw_textured_quad_uv() expects bottom v first (v0), then top v (v1).
        frame.v0 = static_cast<float>(temp_frame.y + temp_frame.height) / static_cast<float>(atlas_h);
        frame.v1 = static_cast<float>(temp_frame.y) / static_cast<float>(atlas_h);
        frame.valid = true;
        frames_.emplace(pending_frame_name, frame);
        ++frame_insert_count;
    };

    for (const auto& raw_line : lines) {
        const std::string line = trim_copy(raw_line);
        if (line.empty()) continue;

        if (line == "<key>texture</key>") { pending_texture_dict = true;  continue; }
        if (line == "<key>frames</key>")  { pending_frames_dict  = true;  continue; }

        if (line == "<dict>") {
            if (pending_texture_dict) { in_texture_dict = true; pending_texture_dict = false; continue; }
            if (pending_frames_dict)  { in_frames_dict  = true; pending_frames_dict  = false; continue; }
            if (in_frames_dict && !in_frame_dict && !pending_frame_name.empty()) {
                in_frame_dict = true;
                pending_field.clear();
                temp_frame = TempFrame{};
                continue;
            }
        }
        if (line == "</dict>") {
            if (in_frame_dict) {
                flush_frame();
                in_frame_dict = false;
                pending_field.clear();
                pending_frame_name.clear();
                continue;
            }
            if (in_texture_dict) { in_texture_dict = false; pending_field.clear(); continue; }
            if (in_frames_dict)  { in_frames_dict  = false; pending_field.clear(); continue; }
        }

        if (in_frames_dict && !in_frame_dict) {
            std::string key;
            if (extract_key_tag(line, key)) pending_frame_name = key;
            continue;
        }

        if (in_texture_dict || in_frame_dict) {
            std::string key;
            if (extract_key_tag(line, key)) { pending_field = key; continue; }

            if (!pending_field.empty()) {
                float value = 0.0f;
                if (!extract_number_tag(line, value)) continue;

                if (in_texture_dict) {
                    if      (pending_field == "width")  texture_width  = static_cast<int>(value);
                    else if (pending_field == "height") texture_height = static_cast<int>(value);
                } else {
                    if      (pending_field == "x")              temp_frame.x               = static_cast<int>(value);
                    else if (pending_field == "y")              temp_frame.y               = static_cast<int>(value);
                    else if (pending_field == "width")          temp_frame.width           = static_cast<int>(value);
                    else if (pending_field == "height")         temp_frame.height          = static_cast<int>(value);
                    else if (pending_field == "offsetX")        temp_frame.offset_x        = value;
                    else if (pending_field == "offsetY")        temp_frame.offset_y        = value;
                    else if (pending_field == "originalWidth")  temp_frame.original_width  = static_cast<int>(value);
                    else if (pending_field == "originalHeight") temp_frame.original_height = static_cast<int>(value);
                }
                pending_field.clear();
            }
        }
    }

    plist_log("parsed plist ok: %s (inserted=%d, duplicates=%d)",
              plist_relative_path.c_str(), frame_insert_count, frame_duplicate_count);
    return true;
}

} // namespace defender
