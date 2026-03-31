#include "scenes/research/research_components.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

#include "engine/font_renderer.hpp"
#include "game/bow_data.hpp"
#include "game/item_cost.hpp"
#include "game/param.hpp"
#include "scenes/research/research_render.hpp"

namespace defender {
namespace {

constexpr int SHOW_MAX_LEVEL = 0;
constexpr int SHOW_LOCKED = 1;
constexpr int SHOW_UPGRADE = 2;
constexpr int SHOW_EQUIP = 3;
constexpr int SHOW_BUY = 4;

constexpr const char* kUpgradeUp = "assets/imgs_480_800/research/button_upgrade_up.png";
constexpr const char* kUpgradeDown = "assets/imgs_480_800/research/button_upgrade_down.png";
constexpr const char* kUpgradeUpKr = "assets/imgs_480_800/research/button_upgrade_up_kr.png";
constexpr const char* kUpgradeDownKr = "assets/imgs_480_800/research/button_upgrade_down_kr.png";
constexpr const char* kBuyUp = "assets/imgs_480_800/research/research_button_buy_up.png";
constexpr const char* kBuyDown = "assets/imgs_480_800/research/research_button_buy_down.png";
constexpr const char* kBuyUpKr = "assets/imgs_480_800/research/research_button_buy_up_kr.png";
constexpr const char* kBuyDownKr = "assets/imgs_480_800/research/research_button_buy_down_kr.png";
constexpr const char* kEquipUp = "assets/imgs_480_800/research/research_button_equip_up.png";
constexpr const char* kEquipDown = "assets/imgs_480_800/research/research_button_equip_down.png";
constexpr const char* kEquipUpKr = "assets/imgs_480_800/research/research_button_equip_up_kr.png";
constexpr const char* kEquipDownKr = "assets/imgs_480_800/research/research_button_equip_down_kr.png";

constexpr const char* kCoinPath = "assets/imgs_480_800/game/coin.png";
constexpr const char* kStonePath = "assets/imgs_480_800/game/mana_stone.png";
constexpr const char* kNumberStrip = "assets/imgs_480_800/game/z_number_list.png";
constexpr float kActionButtonX = 619.0f;
constexpr float kActionButtonY = 25.0f;
constexpr float kUpgradeButtonW = 144.0f;
constexpr float kUpgradeButtonH = 36.0f;
constexpr float kEquipButtonW = 144.0f;
constexpr float kEquipButtonH = 50.0f;
constexpr float kInfoTextX = 618.0f;
constexpr float kInfoTextY = 25.0f;
constexpr float kInfoTextWidth = 145.0f;
constexpr float kInfoTextHeight = 70.0f;
constexpr float kInfoTextFontSize = 17.0f;
constexpr float kInfoTextLineStep = 17.0f;
constexpr float kMaxInfoTextX = 618.0f;
constexpr float kMaxInfoTextY = 36.0f;
constexpr float kMaxInfoTextWidth = 145.0f;
constexpr float kMaxInfoTextHeight = 48.0f;
constexpr float kMaxInfoTextFontSize = 15.0f;
constexpr float kMaxInfoTextLineStep = 15.0f;
constexpr float kCoinW = 26.0f;
constexpr float kCoinH = 27.0f;
constexpr float kStoneW = 27.0f;
constexpr float kStoneH = 30.0f;

constexpr const char* kNeed = "Need";
constexpr const char* kLevel = "Level";
constexpr const char* kLevelMax = "Level Max";
constexpr const char* kCityWall = "City Wall";
constexpr const char* kLavaMoat = "Lava Moat";
constexpr const char* kMagicTower = "Magic Tower";
constexpr const char* kStrength = "Strength";
constexpr const char* kAgility = "Agility";
constexpr const char* kPowerShot = "Power Shot";
constexpr const char* kFatalBlow = "Fatal Blow";
constexpr const char* kMultiArrow = "Multi Arrow";
constexpr const char* kManaResearch = "Mana Research";
constexpr const char* kFireBall = "Fire Ball";
constexpr const char* kMeteor = "Meteor";
constexpr const char* kGlacialSpike = "Glacial Spike";
constexpr const char* kFrostNova = "Frost Nova";
constexpr const char* kLightningStrike = "Lightning Strike";
constexpr const char* kThunderStorm = "Thunder Storm";

UpdateArea::InfoLine white_line(std::string text) {
    return {std::move(text), 1.0f, 1.0f, 1.0f};
}

UpdateArea::InfoLine red_line(std::string text) {
    return {std::move(text), 1.0f, 0.0f, 0.0f};
}

float number_width(int value, float scale) {
    int safe = value;
    if (safe < 0) safe = 0;
    return static_cast<float>(std::to_string(safe).size()) * 25.0f * scale;
}

const char* pick_button(bool down, int show_type) {
    const bool kr = Param::language == 1;
    if (show_type == SHOW_UPGRADE) {
        return down ? (kr ? kUpgradeDownKr : kUpgradeDown) : (kr ? kUpgradeUpKr : kUpgradeUp);
    }
    if (show_type == SHOW_BUY) {
        return down ? (kr ? kBuyDownKr : kBuyDown) : (kr ? kBuyUpKr : kBuyUp);
    }
    return down ? (kr ? kEquipDownKr : kEquipDown) : (kr ? kEquipUpKr : kEquipUp);
}

RectF action_button_rect(int show_type) {
    return make_rect(
        sx(kActionButtonX),
        sy(kActionButtonY),
        sx(show_type == SHOW_EQUIP ? kEquipButtonW : kUpgradeButtonW),
        sy(show_type == SHOW_EQUIP ? kEquipButtonH : kUpgradeButtonH)
    );
}

bool button_contains(float x, float y, int show_type) {
    return action_button_rect(show_type).contains(x, y);
}

void draw_action_button(bool down, int show_type) {
    const char* path = pick_button(down, show_type);
    const RectF rect = action_button_rect(show_type);
    draw_texture(path, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 1.0f);
}

void draw_info_lines(
    const std::vector<UpdateArea::InfoLine>& lines,
    float box_x,
    float box_y,
    float width,
    float height,
    float font_size,
    float line_step
) {
    if (lines.empty()) {
        return;
    }

    struct WrappedInfoLine {
        std::string text;
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
    };

    auto& fonts = FontRenderer::instance();
    const float max_width = sx(width);

    auto split_wrapped_text = [&](const std::string& text) {
        std::vector<std::string> wrapped;
        if (text.empty()) {
            wrapped.emplace_back();
            return wrapped;
        }

        std::vector<std::string> tokens;
        std::string current;
        bool current_is_space = false;
        bool has_current = false;
        for (unsigned char ch : text) {
            const bool is_space = std::isspace(ch) != 0;
            if (!has_current) {
                current.assign(1, static_cast<char>(ch));
                current_is_space = is_space;
                has_current = true;
                continue;
            }
            if (current_is_space == is_space) {
                current.push_back(static_cast<char>(ch));
            } else {
                tokens.push_back(current);
                current.assign(1, static_cast<char>(ch));
                current_is_space = is_space;
            }
        }
        if (!current.empty()) {
            tokens.push_back(current);
        }

        std::string line_text;
        float line_width = 0.0f;
        const float pixel_height = sy(font_size);
        auto measure = [&](const std::string& value) {
            return fonts.measure_text_width(FontFaceId::Ants, value, pixel_height);
        };
        auto flush_line = [&]() {
            wrapped.push_back(line_text);
            line_text.clear();
            line_width = 0.0f;
        };

        for (const auto& token : tokens) {
            if (token.empty()) {
                continue;
            }
            const bool token_is_space = std::all_of(token.begin(), token.end(), [](unsigned char ch) {
                return std::isspace(ch) != 0;
            });
            if (token_is_space) {
                if (line_text.empty()) {
                    continue;
                }
                const float token_width = measure(token);
                if ((line_width + token_width) <= max_width) {
                    line_text += token;
                    line_width += token_width;
                }
                continue;
            }

            std::vector<std::string> pieces;
            if (measure(token) > max_width) {
                std::string chunk;
                for (char ch : token) {
                    const std::string candidate = chunk + ch;
                    if (!chunk.empty() && measure(candidate) > max_width) {
                        pieces.push_back(chunk);
                        chunk.assign(1, ch);
                    } else {
                        chunk.push_back(ch);
                    }
                }
                if (!chunk.empty()) {
                    pieces.push_back(chunk);
                }
            } else {
                pieces.push_back(token);
            }

            for (const auto& piece : pieces) {
                const float piece_width = measure(piece);
                if (!line_text.empty() && (line_width + piece_width) > max_width) {
                    flush_line();
                }
                line_text += piece;
                line_width += piece_width;
            }
        }

        if (line_text.empty() && wrapped.empty()) {
            wrapped.emplace_back();
        } else if (!line_text.empty()) {
            flush_line();
        }
        return wrapped;
    };

    std::vector<WrappedInfoLine> wrapped_lines;
    wrapped_lines.reserve(lines.size());
    for (const auto& line : lines) {
        for (const auto& wrapped : split_wrapped_text(line.text)) {
            wrapped_lines.push_back({wrapped, line.r, line.g, line.b});
        }
    }

    const float total_block_height = (static_cast<float>(wrapped_lines.size()) * font_size) +
                                     (static_cast<float>(std::max(0, static_cast<int>(wrapped_lines.size()) - 1)) * (line_step - font_size));
    const float layout_scale = total_block_height > height
        ? (height / total_block_height)
        : 1.0f;
    const float scaled_font_size = font_size * layout_scale;
    const float scaled_line_step = line_step * layout_scale;
    const float pixel_height = sy(scaled_font_size);
    const float block_height = (static_cast<float>(wrapped_lines.size()) * scaled_font_size) +
                               (static_cast<float>(std::max(0, static_cast<int>(wrapped_lines.size()) - 1)) * (scaled_line_step - scaled_font_size));
    float baseline_y = sy(box_y + std::max(0.0f, (height - block_height) * 0.5f) + scaled_font_size);
    for (const auto& line : wrapped_lines) {
        const float line_width = fonts.measure_text_width(FontFaceId::Ants, line.text, pixel_height);
        const float min_x = sx(box_x);
        const float max_x = sx(box_x + width);
        float draw_x = sx(box_x) + std::max(0.0f, (sx(width) - line_width) * 0.5f);
        if ((draw_x + line_width) > max_x) {
            draw_x = std::max(min_x, max_x - line_width);
        }
        fonts.draw_text(FontFaceId::Ants, line.text, draw_x + sx(1.0f), baseline_y + sy(1.0f), pixel_height, 0.0f, 0.0f, 0.0f, 1.0f);
        fonts.draw_text(FontFaceId::Ants, line.text, draw_x, baseline_y, pixel_height, line.r, line.g, line.b, 1.0f);
        baseline_y += sy(scaled_line_step);
    }
}

} // namespace

UpdateArea::UpdateArea(ItemZone* item_zone) : item_zone_(item_zone) {}

bool UpdateArea::touch(const TouchEvent& event, float x1, float y1, float, float) {
    if (event.action == TouchAction::Down) {
        if (show_type_ >= SHOW_UPGRADE && button_contains(x1, y1, show_type_)) {
            pressed_ = true;
            return false;
        }
        return true;
    }
    if (event.action == TouchAction::Up) {
        if (button_contains(x1, y1, show_type_)) {
            if (show_type_ == SHOW_UPGRADE) {
                item_zone_->upgrade(item_type_, cost_, use_gold_);
            } else if (show_type_ == SHOW_BUY) {
                item_zone_->buy_bow(item_type_, cost_, use_gold_);
            } else if (show_type_ == SHOW_EQUIP) {
                item_zone_->equip_bow(item_type_);
            }
        }
        pressed_ = false;
    }
    return true;
}

std::vector<UpdateArea::InfoLine> UpdateArea::need_lines_for_item(int item_type, bool is_bow) {
    if (is_bow) {
        int temp_level = 2;
        if (item_type % 9 != 1) {
            temp_level = ((item_type - 1) % 9) * 5;
        }
        if (Param::language == 1) {
            return {
                white_line(kNeed),
                red_line(std::string(kLevel) + " " + std::to_string(temp_level))
            };
        }
        return {
            white_line(kNeed),
            red_line(std::string(kLevel) + " " + std::to_string(temp_level)),
            white_line("To Unlock")
        };
    }
    switch (item_type) {
    case 1:
    case 4:
        return {white_line(kNeed), red_line(""), red_line(std::string(kCityWall) + " Lv.3")};
    case 2:
    case 3:
        return {white_line(kNeed), red_line(""), red_line(std::string(kLavaMoat) + " Lv.1")};
    case 5:
    case 6:
        return {white_line(kNeed), red_line(std::string(kMagicTower) + " Lv.1")};
    case 9:
        return {white_line(kNeed), red_line(""), red_line(std::string(kStrength) + " Lv.5")};
    case 10:
        return {white_line(kNeed), red_line(""), red_line(std::string(kAgility) + " Lv.5")};
    case 11:
        return {white_line(kNeed), red_line(std::string(kPowerShot) + " Lv.3"), red_line(std::string(kFatalBlow) + " Lv.3")};
    case 12:
        return {white_line(kNeed), red_line(std::string(kMultiArrow) + " Lv.2")};
    case 13:
        return {white_line(kNeed), red_line(std::string(kStrength) + " Lv.10"), red_line(std::string(kAgility) + " Lv.10")};
    case 16:
        return {white_line(kNeed), red_line(std::string(kManaResearch) + " Lv.4"), red_line(std::string(kFireBall) + " Lv.3")};
    case 17:
        return {white_line(kNeed), red_line(std::string(kManaResearch) + " Lv.6"), red_line(std::string(kMeteor) + " Lv.3")};
    case 18:
    case 21:
        return {white_line(kNeed), red_line(""), red_line(std::string(kManaResearch) + " Lv.2")};
    case 19:
        return {white_line(kNeed), red_line(std::string(kManaResearch) + " Lv.4"), red_line(std::string(kGlacialSpike) + " Lv.3")};
    case 20:
        return {white_line(kNeed), red_line(std::string(kManaResearch) + " Lv.6"), red_line(std::string(kFrostNova) + " Lv.3")};
    case 22:
        return {white_line(kNeed), red_line(std::string(kManaResearch) + " Lv.4"), red_line(std::string(kLightningStrike) + " Lv.3")};
    case 23:
        return {white_line(kNeed), red_line(std::string(kManaResearch) + " Lv.6"), red_line(std::string(kThunderStorm) + " Lv.3")};
    default:
        return {red_line("LOCKED")};
    }
}

void UpdateArea::set_type(int item_type, bool is_bow) {
    item_type_ = item_type;
    is_bow_ = is_bow;
    if (is_bow_) {
        BowButton* bow = item_zone_->get_bow(item_type);
        if (bow == nullptr) return;
        if (bow->is_lock()) {
            show_type_ = SHOW_LOCKED;
            info_lines_ = need_lines_for_item(item_type, true);
            return;
        }
        if (bow->is_get()) {
            show_type_ = SHOW_EQUIP;
            info_lines_.clear();
            return;
        }
        show_type_ = SHOW_BUY;
        cost_ = BowData::get_cost(item_type_);
        use_gold_ = item_type_ < 19;
        info_lines_.clear();
        return;
    }

    ResearchButton* item = item_zone_->get_item(item_type);
    if (item == nullptr) return;
    if (item->is_max_level()) {
        show_type_ = SHOW_MAX_LEVEL;
        info_lines_ = {red_line(kLevelMax)};
        return;
    }
    if (item->is_locked()) {
        show_type_ = SHOW_LOCKED;
        info_lines_ = need_lines_for_item(item_type, false);
        return;
    }
    show_type_ = SHOW_UPGRADE;
    cost_ = ItemCost::get_cost(item_type);
    use_gold_ = ItemCost::is_gold_pay(item_type);
    info_lines_.clear();
}

void UpdateArea::draw() {
    if (is_bow_) {
        if (show_type_ == SHOW_LOCKED) {
            draw_info_lines(info_lines_, kInfoTextX, kInfoTextY, kInfoTextWidth, kInfoTextHeight, kInfoTextFontSize, kInfoTextLineStep);
        } else if (show_type_ == SHOW_BUY) {
            draw_action_button(pressed_, show_type_);
        } else if (show_type_ == SHOW_EQUIP) {
            draw_action_button(pressed_, show_type_);
            return;
        }
    } else {
        if (show_type_ == SHOW_LOCKED) {
            draw_info_lines(info_lines_, kInfoTextX, kInfoTextY, kInfoTextWidth, kInfoTextHeight, kInfoTextFontSize, kInfoTextLineStep);
        } else if (show_type_ == SHOW_MAX_LEVEL) {
            draw_info_lines(
                info_lines_,
                kMaxInfoTextX,
                kMaxInfoTextY,
                kMaxInfoTextWidth,
                kMaxInfoTextHeight,
                kMaxInfoTextFontSize,
                kMaxInfoTextLineStep
            );
        } else if (show_type_ == SHOW_UPGRADE) {
            draw_action_button(pressed_, show_type_);
        }
    }

    if (show_type_ == SHOW_UPGRADE || show_type_ == SHOW_BUY) {
        const std::string cost_icon = use_gold_ ? kCoinPath : kStonePath;
        const float number_scale = 0.6f;
        const float num_w = number_width(cost_, number_scale);
        const float icon_w = use_gold_ ? kCoinW : kStoneW;
        const float icon_h = use_gold_ ? kCoinH : kStoneH;
        const float start_x = sx(695.0f) - (num_w * 0.3f) - icon_w;
        const float start_y = sy(62.0f);
        draw_texture(cost_icon, start_x, start_y, icon_w, icon_h, 1.0f);
        draw_number_strip(
            kNumberStrip,
            cost_,
            start_x + (icon_w * 1.1f),
            start_y + sy(3.0f),
            number_scale,
            0.75f,
            0.75f,
            0.8f,
            1.0f
        );
    }
}

} // namespace defender
