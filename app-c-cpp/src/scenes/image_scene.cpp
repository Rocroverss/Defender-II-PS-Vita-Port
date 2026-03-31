#include "scenes/image_scene.hpp"

#include "engine/render_utils.hpp"
#include "engine/texture_cache.hpp"

namespace defender {

ImageScene::ImageScene(std::string image_path) : image_path_(std::move(image_path)) {}

bool ImageScene::touch(const TouchEvent&) {
    return true;
}

void ImageScene::draw() {
    const auto& tex = TextureCache::instance().get(image_path_);
    if (tex.valid) {
        draw_textured_quad(tex.id, 0.0f, 0.0f, screen_width, screen_height, 1.0f);
        return;
    }

    draw_quad(0.0f, 0.0f, screen_width, screen_height, 0.12f, 0.12f, 0.12f, 1.0f);
}

void ImageScene::update() {}

void ImageScene::reset() {}

} // namespace defender

