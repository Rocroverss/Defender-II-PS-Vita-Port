#include "engine/font_renderer.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <vitaGL.h>

#include "engine/render_utils.hpp"

#if __has_include(<ft2build.h>)
#define DEFENDER_HAS_FREETYPE 1
#include <ft2build.h>
#include FT_FREETYPE_H
#else
#define DEFENDER_HAS_FREETYPE 0
#endif

namespace defender {

namespace {

constexpr const char* kAntsFontPath = "assets/ants.TTF";
constexpr const char* kCooperFontPath = "assets/Cooper.otf";

} // namespace

struct FontRenderer::Impl {
    bool enabled = false;

#if DEFENDER_HAS_FREETYPE
    struct GlyphKey {
        int face_id = 0;
        int pixel_height = 0;
        uint32_t codepoint = 0;

        bool operator==(const GlyphKey& other) const {
            return face_id == other.face_id && pixel_height == other.pixel_height && codepoint == other.codepoint;
        }
    };

    struct GlyphKeyHash {
        size_t operator()(const GlyphKey& key) const {
            size_t h = static_cast<size_t>(key.face_id);
            h = (h * 1315423911u) ^ static_cast<size_t>(key.pixel_height);
            h = (h * 1315423911u) ^ static_cast<size_t>(key.codepoint);
            return h;
        }
    };

    struct GlyphTexture {
        GLuint tex = 0;
        int w = 0;
        int h = 0;
        int bearing_x = 0;
        int bearing_y = 0;
        int advance = 0;
        bool valid = false;
    };

    FT_Library ft = nullptr;
    FT_Face ants = nullptr;
    FT_Face cooper = nullptr;

    std::unordered_map<GlyphKey, GlyphTexture, GlyphKeyHash> glyph_cache;

    static bool try_load_face(FT_Library ft_lib, const char* rel_path, FT_Face* out_face) {
        const std::vector<std::string> candidates = {
            rel_path,
            std::string("app0:/") + rel_path,
            std::string("../") + rel_path
        };
        for (const auto& path : candidates) {
            if (FT_New_Face(ft_lib, path.c_str(), 0, out_face) == 0) {
                return true;
            }
        }
        return false;
    }

    FT_Face resolve_face(FontFaceId font_id) const {
        return font_id == FontFaceId::Cooper ? cooper : ants;
    }

    int resolve_face_key(FontFaceId font_id) const {
        return font_id == FontFaceId::Cooper ? 1 : 0;
    }

    GlyphTexture build_glyph(FT_Face face, uint32_t codepoint, int px_height) {
        GlyphTexture out;
        if (face == nullptr) {
            return out;
        }
        if (FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(px_height)) != 0) {
            return out;
        }
        if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER) != 0) {
            return out;
        }

        FT_GlyphSlot g = face->glyph;
        out.w = static_cast<int>(g->bitmap.width);
        out.h = static_cast<int>(g->bitmap.rows);
        out.bearing_x = g->bitmap_left;
        out.bearing_y = g->bitmap_top;
        out.advance = static_cast<int>(g->advance.x >> 6);

        if (out.w <= 0 || out.h <= 0 || g->bitmap.buffer == nullptr) {
            out.valid = false;
            return out;
        }

        std::vector<unsigned char> rgba(static_cast<size_t>(out.w * out.h * 4), 255);
        for (int y = 0; y < out.h; ++y) {
            for (int x = 0; x < out.w; ++x) {
                const unsigned char a = g->bitmap.buffer[static_cast<size_t>(y * g->bitmap.pitch + x)];
                const size_t idx = static_cast<size_t>((y * out.w + x) * 4);
                rgba[idx + 3] = a;
            }
        }

        GLuint tex = 0;
        glGenTextures(1, &tex);
        if (tex == 0) {
            return out;
        }
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, out.w, out.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

        out.tex = tex;
        out.valid = true;
        return out;
    }

    const GlyphTexture& get_glyph(FontFaceId font_id, uint32_t codepoint, int px_height) {
        const GlyphKey key{resolve_face_key(font_id), px_height, codepoint};
        auto it = glyph_cache.find(key);
        if (it != glyph_cache.end()) {
            return it->second;
        }

        GlyphTexture glyph = build_glyph(resolve_face(font_id), codepoint, px_height);
        auto inserted = glyph_cache.emplace(key, glyph);
        return inserted.first->second;
    }

    void clear_glyphs() {
        for (auto& [_, glyph] : glyph_cache) {
            if (glyph.tex != 0) {
                glDeleteTextures(1, &glyph.tex);
                glyph.tex = 0;
            }
        }
        glyph_cache.clear();
    }
#endif
};

FontRenderer& FontRenderer::instance() {
    static FontRenderer renderer;
    return renderer;
}

FontRenderer::FontRenderer() : impl_(new Impl()) {
#if DEFENDER_HAS_FREETYPE
    if (FT_Init_FreeType(&impl_->ft) != 0) {
        return;
    }

    FT_Face ants_face = nullptr;
    FT_Face cooper_face = nullptr;
    const bool ants_ok = Impl::try_load_face(impl_->ft, kAntsFontPath, &ants_face);
    const bool cooper_ok = Impl::try_load_face(impl_->ft, kCooperFontPath, &cooper_face);
    if (!ants_ok || !cooper_ok) {
        if (ants_face != nullptr) {
            FT_Done_Face(ants_face);
        }
        if (cooper_face != nullptr) {
            FT_Done_Face(cooper_face);
        }
        FT_Done_FreeType(impl_->ft);
        impl_->ft = nullptr;
        return;
    }

    impl_->ants = ants_face;
    impl_->cooper = cooper_face;
    impl_->enabled = true;
#endif
}

FontRenderer::~FontRenderer() {
    if (impl_ == nullptr) {
        return;
    }
#if DEFENDER_HAS_FREETYPE
    impl_->clear_glyphs();
    if (impl_->ants != nullptr) {
        FT_Done_Face(impl_->ants);
        impl_->ants = nullptr;
    }
    if (impl_->cooper != nullptr) {
        FT_Done_Face(impl_->cooper);
        impl_->cooper = nullptr;
    }
    if (impl_->ft != nullptr) {
        FT_Done_FreeType(impl_->ft);
        impl_->ft = nullptr;
    }
#endif
    delete impl_;
    impl_ = nullptr;
}

bool FontRenderer::available() const {
    return impl_ != nullptr && impl_->enabled;
}

void FontRenderer::clear() {
    if (impl_ == nullptr) {
        return;
    }
#if DEFENDER_HAS_FREETYPE
    impl_->clear_glyphs();
#endif
}

void FontRenderer::draw_text(
    FontFaceId font,
    const std::string& text,
    float baseline_x,
    float baseline_y,
    float pixel_height,
    float r,
    float g,
    float b,
    float a
) {
    if (impl_ == nullptr || !impl_->enabled || text.empty() || pixel_height <= 0.0f || a <= 0.0f) {
        return;
    }
#if DEFENDER_HAS_FREETYPE
    const int px = std::max(8, static_cast<int>(pixel_height));
    float pen_x = baseline_x;
    for (unsigned char ch : text) {
        const auto& glyph = impl_->get_glyph(font, static_cast<uint32_t>(ch), px);
        if (glyph.valid && glyph.tex != 0) {
            const float gx = pen_x + static_cast<float>(glyph.bearing_x);
            const float gy = baseline_y + static_cast<float>(glyph.bearing_y - glyph.h);
            draw_textured_quad_tinted(
                glyph.tex,
                gx,
                gy,
                static_cast<float>(glyph.w),
                static_cast<float>(glyph.h),
                r,
                g,
                b,
                a
            );
        }
        pen_x += static_cast<float>(glyph.advance);
    }
#else
    (void)font;
    (void)text;
    (void)baseline_x;
    (void)baseline_y;
    (void)pixel_height;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
#endif
}

float FontRenderer::measure_text_width(
    FontFaceId font,
    const std::string& text,
    float pixel_height
) {
    if (impl_ == nullptr || !impl_->enabled || text.empty() || pixel_height <= 0.0f) {
        return 0.0f;
    }
#if DEFENDER_HAS_FREETYPE
    const int px = std::max(8, static_cast<int>(pixel_height));
    float width = 0.0f;
    for (unsigned char ch : text) {
        const auto& glyph = impl_->get_glyph(font, static_cast<uint32_t>(ch), px);
        width += static_cast<float>(glyph.advance);
    }
    return width;
#else
    (void)font;
    return static_cast<float>(text.size()) * pixel_height * 0.6f;
#endif
}

} // namespace defender
