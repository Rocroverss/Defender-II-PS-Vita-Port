#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "scenes/scene.hpp"

namespace defender {

class ShopScene : public Scene {
public:
    using TransitionRequest = std::function<void(int scene_id, int direction)>;

    struct ShopButton {
        const char* up = nullptr;
        const char* down = nullptr;
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
    };

    explicit ShopScene(TransitionRequest transition_cb);

    bool touch(const TouchEvent& event) override;
    void draw() override;
    void update() override;
    void reset() override;

private:
    static bool hit_test(const ShopButton& button, float px, float py);
    static void draw_button(const ShopButton& button, bool pressed, float alpha = 1.0f);

    void refresh_daily_offer();
    void show_confirmation(int id);
    void claim_reward(int id);
    ShopButton display_button_for_offer(int id) const;

    bool is_offer_button(int id) const;
    bool is_claimed_today() const;
    bool is_claimed_choice(int id) const;
    bool is_gold_offer(int id) const;
    bool touch_confirmation(const TouchEvent& event);

    std::string reward_text(int id) const;

    TransitionRequest transition_cb_;
    int pressed_id_ = -1;
    int confirm_choice_id_ = -1;
    int confirm_pressed_id_ = -1;
    int gold_offer_id_ = 0;
    int stone_offer_id_ = 3;
    int claimed_reward_type_ = 0;
    int64_t offer_day_key_ = -1;
    int64_t claimed_day_key_ = -1;
    bool confirm_visible_ = false;
    float pulse_ = 0.0f;
};

} // namespace defender
