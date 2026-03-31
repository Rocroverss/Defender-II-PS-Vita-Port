#include "scenes/scene.hpp"

namespace defender {

float Scene::screen_width = 0.0f;
float Scene::screen_height = 0.0f;

void Scene::scene_init() {
    auto& mapper = CoordinateMapper::instance();
    screen_width = mapper.gen_game_length_x(800.0f);
    screen_height = mapper.gen_game_length_y(480.0f);
}

void Scene::scene_draw() {
    glPushMatrix();
    glTranslatef(scene_x + (screen_width / 2.0f), scene_y + (screen_height / 2.0f), 0.0f);
    glScalef(scene_scale, scene_scale, 1.0f);
    glTranslatef((-screen_width) / 2.0f, (-screen_height) / 2.0f, 0.0f);
    glColor4f(scene_alpha, scene_alpha, scene_alpha, scene_alpha);
    draw();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();
}

float Scene::get_xy(float xy) {
    return CoordinateMapper::instance().gen_game_length(xy);
}

float Scene::get_x(float x) {
    return CoordinateMapper::instance().gen_game_length_x(x);
}

float Scene::get_y(float y) {
    return CoordinateMapper::instance().gen_game_length_y(y);
}

}

