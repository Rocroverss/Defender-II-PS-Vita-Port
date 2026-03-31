#include "engine/texture_cache.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <setjmp.h>
#include <string>
#include <vector>

#include <jpeglib.h>
#include <png.h>

#include "engine/sprite_atlas_cache.hpp"

namespace defender {

namespace {

struct RawImage {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> rgba;
};

void trim_trailing_transparent_padding(RawImage& image) {
    if (image.width <= 0 || image.height <= 0 || image.rgba.empty()) {
        return;
    }

    int max_x = -1;
    int max_y = -1;
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            const size_t a_idx = static_cast<size_t>((y * image.width + x) * 4 + 3);
            if (image.rgba[a_idx] > 1U) {
                if (x > max_x) max_x = x;
                if (y > max_y) max_y = y;
            }
        }
    }

    if (max_x < 0 || max_y < 0) {
        return;
    }

    const int trimmed_w = max_x + 1;
    const int trimmed_h = max_y + 1;
    if (trimmed_w == image.width && trimmed_h == image.height) {
        return;
    }

    std::vector<unsigned char> trimmed(static_cast<size_t>(trimmed_w * trimmed_h * 4));
    for (int y = 0; y < trimmed_h; ++y) {
        const unsigned char* src = image.rgba.data() + static_cast<size_t>((y * image.width) * 4);
        unsigned char* dst       = trimmed.data()    + static_cast<size_t>((y * trimmed_w)   * 4);
        std::copy(src, src + static_cast<size_t>(trimmed_w * 4), dst);
    }

    image.width  = trimmed_w;
    image.height = trimmed_h;
    image.rgba   = std::move(trimmed);
}

std::string to_lower_copy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string extension_lower(const std::string& path) {
    const size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return {};
    return to_lower_copy(path.substr(dot));
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

std::string to_plist_path(const std::string& texture_path) {
    const size_t dot = texture_path.find_last_of('.');
    if (dot == std::string::npos) return texture_path + ".plist";
    return texture_path.substr(0, dot) + ".plist";
}

bool file_exists(const std::string& path) {
    FILE* fp = std::fopen(path.c_str(), "rb");
    if (fp == nullptr) return false;
    std::fclose(fp);
    return true;
}

bool has_suffix_ignore_case(const std::string& text, const char* suffix_lower) {
    const size_t suffix_len = std::strlen(suffix_lower);
    if (text.size() < suffix_len) return false;
    const std::string tail = to_lower_copy(text.substr(text.size() - suffix_len));
    return tail == suffix_lower;
}

std::string fixed_variant_for_candidate_path(const std::string& candidate_path) {
    const std::string lower = to_lower_copy(candidate_path);
    // Atlas textures must keep exact dimensions for plist UV coordinates.
    if (is_always_plist_texture_path_lower(lower)) {
        return candidate_path;
    }

    const std::string ext = extension_lower(candidate_path);
    if (ext != ".png" && ext != ".jpg" && ext != ".jpeg") {
        return candidate_path;
    }

    if (has_suffix_ignore_case(candidate_path, "_fixed.png") ||
        has_suffix_ignore_case(candidate_path, "_fixed.jpg") ||
        has_suffix_ignore_case(candidate_path, "_fixed.jpeg")) {
        return candidate_path;
    }

    const size_t dot = candidate_path.find_last_of('.');
    if (dot == std::string::npos) {
        return candidate_path;
    }

    const std::string fixed_candidate =
        candidate_path.substr(0, dot) + "_fixed" + candidate_path.substr(dot);
    if (file_exists(fixed_candidate)) {
        return fixed_candidate;
    }
    return candidate_path;
}

std::vector<std::string> plist_candidates_for(const std::string& texture_path) {
    const std::string plist = to_plist_path(texture_path);
    std::vector<std::string> v;
    v.push_back(plist);
    if (plist.rfind("app0:/", 0) != 0) {
        v.push_back("app0:/" + plist);
    }
    v.push_back("../" + plist);
    return v;
}

std::string find_companion_plist(const std::string& texture_path) {
    const std::string lower = to_lower_copy(texture_path);
    if (!is_always_plist_texture_path_lower(lower)) {
        return {};
    }
    for (const auto& c : plist_candidates_for(texture_path)) {
        if (file_exists(c)) return c;
    }
    return {};
}

bool should_trim_transparent_padding(const std::string& path) {
    const std::string lower = to_lower_copy(path);
    if (is_always_plist_texture_path_lower(lower)) {
        return false;
    }
    if (lower.find("assets/imgs_480_800/game/") != std::string::npos) {
        return false;
    }
    if (lower.find("assets/imgs_480_800/research/equip_cover_bg.png") != std::string::npos ||
        lower.find("assets/imgs_480_800/research/equip_cover_bg_fixed.png") != std::string::npos ||
        lower.find("assets/imgs_480_800/research/research_line.png") != std::string::npos ||
        lower.find("assets/imgs_480_800/research/research_defender_line.png") != std::string::npos ||
        lower.find("assets/imgs_480_800/research/research_magic_line.png") != std::string::npos ||
        lower.find("assets/imgs_480_800/research/research_wall_line.png") != std::string::npos) {
        return false;
    }
    return true;
}

std::string normalize_asset_path(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    path = to_lower_copy(path);
    if (path.rfind("app0:/", 0) == 0) {
        path.erase(0, 6);
    }
    while (path.rfind("../", 0) == 0) {
        path.erase(0, 3);
    }
    while (path.rfind("./", 0) == 0) {
        path.erase(0, 2);
    }
    return path;
}

bool should_crop_to_800x400(const std::string& full_path) {
    (void)full_path;
    return false;
}

void crop_center(RawImage& image, int target_width, int target_height) {
    if (image.width <= 0 || image.height <= 0 || image.rgba.empty()) {
        return;
    }
    if (image.width < target_width || image.height < target_height) {
        return;
    }
    if (image.width == target_width && image.height == target_height) {
        return;
    }

    const int crop_x = (image.width - target_width) / 2;
    const int crop_y = (image.height - target_height) / 2;
    std::vector<unsigned char> cropped(static_cast<size_t>(target_width * target_height * 4));
    for (int y = 0; y < target_height; ++y) {
        const unsigned char* src = image.rgba.data() +
            static_cast<size_t>(((crop_y + y) * image.width + crop_x) * 4);
        unsigned char* dst = cropped.data() + static_cast<size_t>(y * target_width * 4);
        std::copy(src, src + static_cast<size_t>(target_width * 4), dst);
    }

    image.width = target_width;
    image.height = target_height;
    image.rgba = std::move(cropped);
}

bool decode_png_rgba(const std::string& path, RawImage& out) {
    FILE* fp = std::fopen(path.c_str(), "rb");
    if (fp == nullptr) return false;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr) { std::fclose(fp); return false; }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        std::fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr)) != 0) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        std::fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width = 0, height = 0;
    int bit_depth = 0, color_type = 0;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                 nullptr, nullptr, nullptr);

    if (bit_depth == 16)                                              png_set_strip_16(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE)                         png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)           png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0)         png_set_tRNS_to_alpha(png_ptr);
    if (color_type == PNG_COLOR_TYPE_RGB   ||
        color_type == PNG_COLOR_TYPE_GRAY  ||
        color_type == PNG_COLOR_TYPE_PALETTE)                         png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY  ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)                      png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    out.width  = static_cast<int>(width);
    out.height = static_cast<int>(height);
    out.rgba.resize(static_cast<size_t>(out.width * out.height * 4));

    std::vector<png_bytep> rows(static_cast<size_t>(out.height));
    for (int y = 0; y < out.height; ++y)
        rows[static_cast<size_t>(y)] = out.rgba.data() + static_cast<size_t>(y * out.width * 4);
    png_read_image(png_ptr, rows.data());

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    std::fclose(fp);

    // BUG FIX: pass `path` (the resolved full path) so that
    // should_trim_transparent_padding tests the correct location for a
    // companion .plist, not some other candidate path.
    if (should_trim_transparent_padding(path)) {
        trim_trailing_transparent_padding(out);
    }
    return true;
}

struct JpegErr {
    jpeg_error_mgr pub;
    jmp_buf setjmp_buf;
};

void jpeg_error_exit(j_common_ptr cinfo) {
    JpegErr* err = reinterpret_cast<JpegErr*>(cinfo->err);
    longjmp(err->setjmp_buf, 1);
}

bool decode_jpeg_rgba(const std::string& path, RawImage& out) {
    FILE* fp = std::fopen(path.c_str(), "rb");
    if (fp == nullptr) return false;

    jpeg_decompress_struct cinfo{};
    JpegErr jerr{};
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpeg_error_exit;

    if (setjmp(jerr.setjmp_buf) != 0) {
        jpeg_destroy_decompress(&cinfo);
        std::fclose(fp);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    out.width  = static_cast<int>(cinfo.output_width);
    out.height = static_cast<int>(cinfo.output_height);
    out.rgba.resize(static_cast<size_t>(out.width * out.height * 4));

    const int channels = static_cast<int>(cinfo.output_components);
    std::vector<unsigned char> scanline(static_cast<size_t>(out.width * channels));
    while (cinfo.output_scanline < cinfo.output_height) {
        JSAMPROW row_ptr = scanline.data();
        jpeg_read_scanlines(&cinfo, &row_ptr, 1);

        const int y = static_cast<int>(cinfo.output_scanline - 1);
        unsigned char* dst = out.rgba.data() + static_cast<size_t>(y * out.width * 4);
        for (int x = 0; x < out.width; ++x) {
            const int src_i = x * channels;
            const int dst_i = x * 4;
            const unsigned char r = scanline[static_cast<size_t>(src_i)];
            const unsigned char g = channels > 1 ? scanline[static_cast<size_t>(src_i + 1)] : r;
            const unsigned char b = channels > 2 ? scanline[static_cast<size_t>(src_i + 2)] : r;
            dst[static_cast<size_t>(dst_i)]     = r;
            dst[static_cast<size_t>(dst_i + 1)] = g;
            dst[static_cast<size_t>(dst_i + 2)] = b;
            dst[static_cast<size_t>(dst_i + 3)] = 255;
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    std::fclose(fp);
    return true;
}

TextureHandle upload_texture_rgba(const RawImage& image) {
    TextureHandle out;
    if (image.rgba.empty() || image.width <= 0 || image.height <= 0) return out;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    if (tex == 0) return out;

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image.width, image.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());

    out.id     = tex;
    out.width  = image.width;
    out.height = image.height;
    out.valid  = true;
    return out;
}

} // namespace

// ---------------------------------------------------------------------------
// TextureCache
// ---------------------------------------------------------------------------

TextureCache& TextureCache::instance() {
    static TextureCache cache;
    return cache;
}

const TextureHandle& TextureCache::get(const std::string& relative_path) {
    auto it = cache_.find(relative_path);
    if (it != cache_.end()) return it->second;

    std::string resolved_path;
    TextureHandle loaded = load_texture_with_fallbacks(relative_path, resolved_path);

    auto inserted = cache_.emplace(relative_path, loaded);
    if (inserted.first->second.valid) {
        const std::string ext = extension_lower(relative_path);
        if (ext == ".png") {
            const std::string companion_plist = find_companion_plist(resolved_path);
            if (!companion_plist.empty()) {
                // Pass resolved texture path so atlas loader uses matching prefix.
                SpriteAtlasCache::instance().ensure_companion_plist_loaded(resolved_path);
            }
        }
    }
    return inserted.first->second;
}

void TextureCache::clear() {
    for (auto& [_, tex] : cache_) {
        if (tex.valid && tex.id != 0) {
            glDeleteTextures(1, &tex.id);
            tex.id    = 0;
            tex.valid = false;
        }
    }
    cache_.clear();
}

TextureHandle TextureCache::load_texture_with_fallbacks(const std::string& relative_path,
                                                         std::string& out_resolved_path) const {
    auto try_candidate = [&out_resolved_path](const std::string& candidate) -> TextureHandle {
        const std::string preferred = fixed_variant_for_candidate_path(candidate);
        if (preferred != candidate) {
            TextureHandle fixed_tex = load_texture_file(preferred);
            if (fixed_tex.valid) {
                out_resolved_path = preferred;
                return fixed_tex;
            }
        }

        TextureHandle original_tex = load_texture_file(candidate);
        if (original_tex.valid) {
            out_resolved_path = candidate;
            return original_tex;
        }
        return {};
    };

    TextureHandle tex = try_candidate(relative_path);
    if (tex.valid) return tex;

    if (relative_path.rfind("app0:/", 0) != 0) {
        const std::string prefixed = "app0:/" + relative_path;
        tex = try_candidate(prefixed);
        if (tex.valid) return tex;
    }

    const std::string dotdot = "../" + relative_path;
    tex = try_candidate(dotdot);
    if (tex.valid) return tex;

    out_resolved_path = relative_path;
    return {};
}

TextureHandle TextureCache::load_texture_file(const std::string& full_path) {
    const std::string ext = extension_lower(full_path);

    RawImage image;
    bool ok = false;
    if      (ext == ".png")              ok = decode_png_rgba(full_path, image);
    else if (ext == ".jpg" || ext == ".jpeg") ok = decode_jpeg_rgba(full_path, image);

    if (!ok) return {};
    if (should_crop_to_800x400(full_path)) {
        crop_center(image, 800, 400);
    }
    return upload_texture_rgba(image);
}

} // namespace defender
