#pragma once

#include <functional>

#include "scenes/scene.hpp"
#include "scenes/research/research_components.hpp"

namespace defender {

class ResearchScene : public Scene {
public:
    using TransitionRequest = std::function<void(int scene_id, int direction)>;

    explicit ResearchScene(TransitionRequest transition_cb);

    bool touch(const TouchEvent& event) override;
    void draw() override;
    void update() override;
    void reset() override;

private:
    void show_bow_menu();
    void set_menu(int menu_id);

    TransitionRequest transition_cb_;
    EquipZone equip_zone_;
    ItemZone item_zone_;
    int pressed_menu_ = 0;
    bool continue_pressed_ = false;
    bool shop_pressed_ = false;
    bool bow_flash_flag_ = false;
    int show_gold_ = 0;
    int show_stone_ = 0;
};

} // namespace defender
