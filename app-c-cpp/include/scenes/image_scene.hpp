#pragma once

#include <string>

#include "scenes/scene.hpp"

namespace defender {

class ImageScene : public Scene {
public:
    explicit ImageScene(std::string image_path);

    bool touch(const TouchEvent& event) override;
    void draw() override;
    void update() override;
    void reset() override;

private:
    std::string image_path_;
};

} // namespace defender

