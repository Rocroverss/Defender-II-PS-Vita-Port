#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "engine/touch_event.hpp"

namespace defender {

struct ResearchLayoutTweak {
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    float scale = 1.0f;
};

enum class ResearchLayoutElement : int {
    CurrentStage = 0,
    GoldAmount,
    EquipCover,
    ResearchLine,
    DefenderLine,
    MagicLine,
    WallLine,
    DescriptionText,
    Count
};

// Editable research screen layout tweaks.
inline std::array<ResearchLayoutTweak, static_cast<int>(ResearchLayoutElement::Count)> kResearchLayoutTweaks = {{
    {-15.0f, -20.0f, 1.0f}, // Current stage
    {0.0f, -23.0f, 1.0f}, // Gold amount
    {0.0f, -10.0f, 1.0f}, // equip_cover_bg_fixed.png
    {0.0f, 0.0f, 1.0f}, // research_line.png
    {0.0f, 0.0f, 1.0f}, // research_defender_line.png
    {0.0f, 0.0f, 1.0f}, // research_magic_line.png
    {0.0f, 0.0f, 1.0f}, // research_wall_line.png
    {0.0f, -10.0f, 1.0f}, // description text
}};

inline const ResearchLayoutTweak& research_layout_tweak(ResearchLayoutElement element) {
    return kResearchLayoutTweaks[static_cast<int>(element)];
}

class BowButton {
public:
    BowButton(const std::string& frame_name, float x, float y);

    void draw(float offset_x, float offset_y, float show_x) const;
    bool contains(float local_x, float local_y) const;

    void press();
    void release();
    bool is_pressed() const;

    void set_get(bool is_get);
    bool is_get() const;

    void set_lock(bool is_lock);
    bool is_lock() const;

private:
    std::string frame_name_;
    float x_ = 0.0f;
    float y_ = 0.0f;
    float w_ = 0.0f;
    float h_ = 0.0f;
    bool is_get_ = false;
    bool is_lock_ = false;
    bool is_select_ = false;
};

class ResearchButton {
public:
    ResearchButton(const std::string& frame_name, float x, float y, int max_level);

    bool is_max_level() const;
    void set_add_level(int level);
    int get_add_level() const;
    void set_level(int level);
    int get_level() const;
    void set_lock(bool is_lock);
    bool is_locked() const;
    bool is_show_upgrade() const;
    void upgrade();
    void set_frame(const std::string& frame_name);

    void press();
    void release();
    bool contains(float local_x, float local_y) const;
    void draw(float offset_x, float offset_y, float show_x);

private:
    std::string frame_name_;
    float x_ = 0.0f;
    float y_ = 0.0f;
    float w_ = 0.0f;
    float h_ = 0.0f;
    int max_level_ = 0;
    int level_ = 0;
    int add_level_ = 0;
    int show_time_ms_ = 0;
    bool is_select_ = false;
    bool is_locked_ = false;
};

class EquipButton {
public:
    EquipButton(int img_id, float x, float y, bool is_bow = true);

    void set_lock(bool is_locked);
    bool is_locked() const;
    void set_scale(float scale);
    bool is_show_equip() const;
    void equip_magic(int type, int level);
    void equip_bow(int bow_id);
    bool is_pressed() const;
    void press();
    void release();
    bool contains(float x, float y) const;
    void draw();

private:
    void update_rect();

    std::string frame_name_;
    float x_ = 0.0f;
    float y_ = 0.0f;
    float w_ = 0.0f;
    float h_ = 0.0f;
    float scale_ = 1.0f;
    int show_time_ms_ = 0;
    bool is_locked_ = false;
    bool is_pressed_ = false;
    bool is_bow_ = true;
    int img_id_ = 0;
};

class EquipZone {
public:
    using ShowBowMenuRequest = std::function<void()>;

    explicit EquipZone(ShowBowMenuRequest show_bow_menu_cb);

    void reset_magic_lock();
    void reset();
    void equip_bow(int type);
    void equip_magic(int type, int level);

    bool touch(const TouchEvent& event, float x1, float y1, float x2, float y2);
    void draw();

private:
    ShowBowMenuRequest show_bow_menu_cb_;
    EquipButton bow_;
    std::array<EquipButton, 3> magic_;
    std::array<EquipButton, 3> fire_;
    std::array<EquipButton, 3> ice_;
    std::array<EquipButton, 3> light_;
    std::array<std::array<EquipButton*, 3>, 3> magic_list_{};
    int press_id_ = 0;
};

class ItemZone;

class Description {
public:
    explicit Description(ItemZone* zone);
    void set_des_type(int type, bool is_bow);
    void draw();

    struct Segment {
        std::string text;
        float size = 16.0f;
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        bool shadow = false;
    };

    using Line = std::vector<Segment>;

private:
    void clear_layout();
    void add_line(std::initializer_list<Segment> segments, float line_height = 0.0f);
    void add_blank_line(float line_height);
    void refresh_text();

    ItemZone* zone_ = nullptr;
    bool is_bow_des_ = false;
    bool refresh_flag_ = false;
    int type_ = 0;
    std::vector<Line> lines_;
    std::vector<float> line_heights_;
};

class UpdateArea {
public:
    struct InfoLine {
        std::string text;
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
    };

    explicit UpdateArea(ItemZone* item_zone);
    bool touch(const TouchEvent& event, float x1, float y1, float x2, float y2);
    void set_type(int item_type, bool is_bow);
    void draw();

private:
    static std::vector<InfoLine> need_lines_for_item(int item_type, bool is_bow);

    ItemZone* item_zone_ = nullptr;
    int item_type_ = 0;
    int show_type_ = 2;
    int cost_ = 0;
    bool is_bow_ = false;
    bool use_gold_ = true;
    bool pressed_ = false;
    std::vector<InfoLine> info_lines_;
};

class ItemZone {
public:
    using OpenShopRequest = std::function<void()>;
    using BowFlashRequest = std::function<void()>;

    ItemZone(EquipZone* equip_zone, OpenShopRequest open_shop_cb, BowFlashRequest bow_flash_cb);

    ResearchButton* get_item(int type);
    BowButton* get_bow(int type);
    int get_add_level(int type) const;

    void set_item_type(int type);
    void reset();
    bool touch(const TouchEvent& event, float x1, float y1, float x2, float y2);
    void update();
    void draw();

    void equip_bow(int type);
    void get_pack_final_fantasy_bow();
    void buy_bow(int type, int cost, bool is_gold);
    void upgrade(int type, int cost, bool is_gold);

private:
    void set_item_type_in_draw(int type);
    void set_target_id(int id);
    void add_show_x(float x);
    void bow_reset();
    void unlock_check();
    void cost_act(int cost, bool is_coin);

    OpenShopRequest open_shop_cb_;
    BowFlashRequest bow_flash_cb_;
    EquipZone* equip_zone_ = nullptr;
    Description des_;
    UpdateArea up_area_;

    static constexpr int ITEM_SIZE = 24;
    static constexpr int BOW_SIZE = 29;

    std::array<std::unique_ptr<ResearchButton>, ITEM_SIZE> item_list_{};
    std::array<std::unique_ptr<BowButton>, BOW_SIZE> bow_list_{};

    float origin_x_ = 0.0f;
    float origin_y_ = 0.0f;
    float show_x_ = 0.0f;
    float show_x_min_ = 0.0f;
    float show_x_max_ = 0.0f;
    float move_speed_ = 0.0f;
    float cur_x_ = 0.0f;
    float pas_x_ = 0.0f;
    int item_type_ = 0;
    int target_type_ = 0;
    int show_start_id_ = 0;
    int show_end_id_ = 0;
    int press_id_ = 0;
    int bow_press_id_ = 0;
    bool move_flag_ = false;
    uint64_t pas_time_ms_ = 0;
    uint64_t cur_time_ms_ = 0;
};

} // namespace defender
