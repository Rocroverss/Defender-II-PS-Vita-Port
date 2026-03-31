#include "scenes/research/research_components.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <initializer_list>
#include <string>
#include <utility>

#include "engine/font_renderer.hpp"
#include "game/bow_data.hpp"
#include "game/item_param.hpp"
#include "game/param.hpp"
#include "game/skill_data.hpp"
#include "scenes/research/research_render.hpp"

namespace defender {
namespace {

struct ColorRgb {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
};

constexpr ColorRgb rgb(int r, int g, int b) {
    return {
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f
    };
}

constexpr ColorRgb kWhite = rgb(255, 255, 255);
constexpr ColorRgb kRed = rgb(255, 0, 0);
constexpr ColorRgb kGreen = rgb(0, 255, 0);
constexpr ColorRgb kYellow = rgb(255, 255, 0);
constexpr ColorRgb kBlue = rgb(0, 0, 255);
constexpr ColorRgb kMagenta = rgb(255, 0, 255);
constexpr ColorRgb kDesc = rgb(162, 164, 181);
constexpr ColorRgb kTowerIce = rgb(12, 198, 237);
constexpr ColorRgb kPhantom = rgb(80, 198, 237);
constexpr ColorRgb kLight = rgb(117, 114, 233);
constexpr ColorRgb kFinal = rgb(255, 155, 0);
constexpr ColorRgb kShadow = rgb(0, 0, 0);

constexpr float kBaseX = 100.0f;
constexpr float kBaseY = 22.0f;
constexpr float kWrapWidth = 505.0f;
constexpr float kWrapHeight = 75.0f;
constexpr float kShadowOffset = 1.0f;

constexpr const char* kBowNormalName = "Light Crossbow";
constexpr const char* kBowNormalDesc = "Newbie crossbow.";
constexpr const char* kBowVolName = "Volcano Lv.";
constexpr const char* kBowVolDesc = "A crossbow with great power.";
constexpr const char* kBowHurName = "Hurricane Lv.";
constexpr const char* kBowHurDesc = "A crossbow with high attack speed.";
constexpr const char* kBowPhaName = "Phantom Lv.";
constexpr const char* kBowPhaDesc = "A crossbow with mysterious force.";
constexpr const char* kBowFfName = "Final Fantasy";
constexpr const char* kBowFfDesc = "The legendary ultimate bow.";
constexpr const char* kBowFfDesc2 = "(Auto-upgrade)";
constexpr const char* kBowAbilityCurrent = "Current Ability:";

constexpr const char* kCurrent = "Current:";
constexpr const char* kNext = "Next:";
constexpr const char* kStrength = "Strength";
constexpr const char* kAgility = "Agility";
constexpr const char* kPowerShot = "Power Shot";
constexpr const char* kFatalBlow = "Fatal Blow";
constexpr const char* kMultiArrow = "Multi Arrow";
constexpr const char* kMultiArrowPlural = "Arrows";
constexpr const char* kDamage = "Damage";
constexpr const char* kPoisonArrow = "Poison Arrow";
constexpr const char* kSeniorHunter = "Senior Hunter";
constexpr const char* kManaResearch = "Mana Research";
constexpr const char* kCityWall = "City Wall";
constexpr const char* kCityWallDesc = "Increase your life points.";
constexpr const char* kLavaMoat = "Lava Moat";
constexpr const char* kLavaMoatDesc = "Lava in front of your citywall.";
constexpr const char* kLavaMoatUpgrade = "Upgrade: Increase lava moat's width.";
constexpr const char* kBurn = "Burn";
constexpr const char* kBurnDesc = "Increase your lava's damage.";
constexpr const char* kEntanglingLava = "Entangling Lava";
constexpr const char* kEntanglingLavaDesc = "Lava moat can decrease Monster's speed";
constexpr const char* kMagicTower = "Magic Tower";
constexpr const char* kMagicTowerDesc = "Attack monsters automatically.";
constexpr const char* kMagicTowerUpgrade = "Upgrade: Increase tower's amount.";
constexpr const char* kMagicPower = "Magic Power";
constexpr const char* kMagicPowerDesc = "Increase your tower's damage.";
constexpr const char* kSplash = "Splash";
constexpr const char* kSplashDesc = "Tower's attack causes splash damage.";
constexpr const char* kStrengthDesc = "Increase your arrow's base damage.";
constexpr const char* kAgilityDesc = "Increase your bow's firing rate.";
constexpr const char* kPowerShotDesc = "Your arrow can repel monster.";
constexpr const char* kPowerShotDesc2 = "Upgrade to improve distance.";
constexpr const char* kFatalBlowDesc = "Your bow may cause double damage.";
constexpr const char* kMultiArrowDesc = "Your bow can shoot multiple arrows.";
constexpr const char* kSeniorHunterDesc = "Give you extra XP.";
constexpr const char* kPoisonArrowDesc = "Poison will decrease monster's attack speed.";
constexpr const char* kManaResearchDesc = "Increase your mana.";
constexpr const char* kFireBall = "Fire Ball";
constexpr const char* kFireBallDesc = "Primary Fire Spells.Burn monsters.";
constexpr const char* kMeteor = "Meteor";
constexpr const char* kMeteorDesc = "Intermediate Fire Spells.Burn monsters.";
constexpr const char* kArmageddon = "Armageddon";
constexpr const char* kArmageddonDesc = "Advanced Fire Spells.Burn monsters.";
constexpr const char* kFireUpgrade = "Upgrade: Increase Damage and Burn time.";
constexpr const char* kGlacialSpike = "Glacial Spike";
constexpr const char* kGlacialSpikeDesc = "Primary Ice Spells.Freeze monsters.";
constexpr const char* kFrostNova = "Frost Nova";
constexpr const char* kFrostNovaDesc = "Intermediate Ice Spells.Freeze monsters.";
constexpr const char* kIceAge = "Ice Age";
constexpr const char* kIceAgeDesc = "Advanced Ice Spells.Freeze monsters.";
constexpr const char* kIceUpgrade = "Upgrade: Increase Damage and Freeze time.";
constexpr const char* kLightningStrike = "Lightning Strike";
constexpr const char* kLightningStrikeDesc = "Primary Lightning Spells.Paralyze monsters.";
constexpr const char* kThunderStorm = "Thunder Storm";
constexpr const char* kThunderStormDesc = "Intermediate Spells.Paralyze monsters.";
constexpr const char* kRagnarok = "Ragnarok";
constexpr const char* kRagnarokDesc = "Advanced Lightning Spells.Paralyze monsters.";
constexpr const char* kLightUpgrade = "Upgrade: Increase Damage and Paralyze time.";

Description::Segment make_segment(const std::string& text, float size, ColorRgb color, bool shadow = false) {
    Description::Segment segment;
    segment.text = text;
    segment.size = size;
    segment.r = color.r;
    segment.g = color.g;
    segment.b = color.b;
    segment.shadow = shadow;
    return segment;
}

Description::Segment title_segment(const std::string& text, float size, ColorRgb color) {
    return make_segment(text, size, color, true);
}

Description::Segment text_segment(const std::string& text, float size, ColorRgb color) {
    return make_segment(text, size, color, false);
}

float segment_width(FontRenderer& fonts, const Description::Segment& segment) {
    return fonts.measure_text_width(FontFaceId::Ants, segment.text, sy(segment.size));
}

bool is_space_only(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    for (const unsigned char ch : text) {
        if (!std::isspace(ch)) {
            return false;
        }
    }
    return true;
}

std::vector<Description::Segment> split_segment_tokens(const Description::Segment& segment) {
    std::vector<Description::Segment> tokens;
    std::string current;
    bool current_is_space = false;
    bool has_current = false;

    auto flush_current = [&]() {
        if (current.empty()) {
            return;
        }
        Description::Segment token = segment;
        token.text = current;
        tokens.push_back(std::move(token));
        current.clear();
        has_current = false;
    };

    for (const unsigned char ch : segment.text) {
        const bool ch_is_space = std::isspace(ch) != 0;
        if (!has_current) {
            current.push_back(static_cast<char>(ch));
            current_is_space = ch_is_space;
            has_current = true;
            continue;
        }
        if (ch_is_space == current_is_space) {
            current.push_back(static_cast<char>(ch));
            continue;
        }
        flush_current();
        current.push_back(static_cast<char>(ch));
        current_is_space = ch_is_space;
        has_current = true;
    }
    flush_current();
    return tokens;
}

std::vector<Description::Line> wrap_line_segments(
    FontRenderer& fonts,
    const Description::Line& line,
    float max_width
) {
    std::vector<Description::Line> wrapped_lines;
    wrapped_lines.emplace_back();
    float current_width = 0.0f;

    auto start_new_line = [&]() {
        if (!wrapped_lines.back().empty()) {
            wrapped_lines.emplace_back();
        }
        current_width = 0.0f;
    };

    auto append_segment = [&](const Description::Segment& token, float token_width) {
        wrapped_lines.back().push_back(token);
        current_width += token_width;
    };

    auto append_wrapped_token = [&](const Description::Segment& token) {
        const bool token_is_space = is_space_only(token.text);
        const float token_width = segment_width(fonts, token);
        if (token_is_space) {
            if (wrapped_lines.back().empty()) {
                return;
            }
            if ((current_width + token_width) <= max_width) {
                append_segment(token, token_width);
            }
            return;
        }

        if (!wrapped_lines.back().empty() && (current_width + token_width) > max_width) {
            start_new_line();
        }
        append_segment(token, token_width);
    };

    for (const auto& segment : line) {
        for (const auto& token : split_segment_tokens(segment)) {
            if (token.text.empty()) {
                continue;
            }

            const float token_width = segment_width(fonts, token);
            if (!is_space_only(token.text) && token_width > max_width) {
                std::string chunk;
                for (char ch : token.text) {
                    Description::Segment next = token;
                    next.text = chunk + ch;
                    if (!chunk.empty() && segment_width(fonts, next) > max_width) {
                        Description::Segment piece = token;
                        piece.text = chunk;
                        append_wrapped_token(piece);
                        chunk.assign(1, ch);
                    } else {
                        chunk.push_back(ch);
                    }
                }
                if (!chunk.empty()) {
                    Description::Segment piece = token;
                    piece.text = chunk;
                    append_wrapped_token(piece);
                }
                continue;
            }

            append_wrapped_token(token);
        }
    }

    while (!wrapped_lines.empty() && wrapped_lines.back().empty()) {
        wrapped_lines.pop_back();
    }
    if (wrapped_lines.empty()) {
        wrapped_lines.emplace_back();
    }
    return wrapped_lines;
}

void draw_line_segments(
    FontRenderer& fonts,
    const Description::Line& line,
    float draw_y,
    float draw_x,
    float scale
) {
    float max_size = 0.0f;
    for (const auto& segment : line) {
        max_size = std::max(max_size, segment.size);
    }

    float pen_x = draw_x;
    const float baseline_y = draw_y + sy(max_size * scale);
    for (const auto& segment : line) {
        const float pixel_height = sy(segment.size * scale);
        if (segment.shadow) {
            fonts.draw_text(
                FontFaceId::Ants,
                segment.text,
                pen_x + sx(kShadowOffset),
                baseline_y + sy(kShadowOffset),
                pixel_height,
                kShadow.r,
                kShadow.g,
                kShadow.b,
                1.0f
            );
        }
        fonts.draw_text(
            FontFaceId::Ants,
            segment.text,
            pen_x,
            baseline_y,
            pixel_height,
            segment.r,
            segment.g,
            segment.b,
            1.0f
        );
        pen_x += fonts.measure_text_width(FontFaceId::Ants, segment.text, pixel_height);
    }
}

std::string value_text(int value, bool percent = false) {
    return std::to_string(value) + (percent ? "%" : "");
}

} // namespace

Description::Description(ItemZone* zone) : zone_(zone) {}

void Description::set_des_type(int type, bool is_bow) {
    is_bow_des_ = is_bow;
    type_ = type;
    refresh_flag_ = true;
}

void Description::clear_layout() {
    lines_.clear();
    line_heights_.clear();
}

void Description::add_line(std::initializer_list<Segment> segments, float line_height) {
    lines_.emplace_back(segments);
    line_heights_.push_back(line_height);
}

void Description::add_blank_line(float line_height) {
    lines_.emplace_back();
    line_heights_.push_back(line_height);
}

void Description::refresh_text() {
    clear_layout();

    auto add_value_line = [this](int current, int next, bool percent = false) {
        add_line(
            {
                text_segment(kCurrent, 20.0f, kYellow),
                text_segment(" " + value_text(current, percent), 20.0f, kWhite),
                text_segment("    ", 20.0f, kYellow),
                text_segment(kNext, 20.0f, kYellow),
                text_segment(" " + value_text(next, percent), 20.0f, kWhite)
            },
            24.0f
        );
    };

    if (is_bow_des_) {
        if (type_ < 0 || type_ > 28) {
            return;
        }

        if (type_ == 0) {
            add_line({title_segment(kBowNormalName, 21.0f, kWhite)}, 25.0f);
            add_blank_line(17.0f);
            add_line({text_segment(kBowNormalDesc, 17.0f, kDesc)}, 21.0f);
            return;
        }

        if (type_ >= 1 && type_ <= 9) {
            const int level = type_;
            const int str = BowData::get_ability(type_, 0);
            const int power = BowData::get_ability(type_, 2);
            add_line({title_segment(std::string(kBowVolName) + std::to_string(level), 21.0f, kRed)}, 25.0f);
            add_line({text_segment(kBowVolDesc, 17.0f, kDesc)}, 21.0f);
            if (power > 0) {
                add_line(
                    {
                        text_segment(kStrength, 17.0f, kRed),
                        text_segment(" +" + std::to_string(str) + ", ", 17.0f, kWhite),
                        text_segment(kPowerShot, 17.0f, kYellow),
                        text_segment(" +" + std::to_string(power), 17.0f, kWhite)
                    },
                    21.0f
                );
            } else {
                add_line(
                    {
                        text_segment(kStrength, 17.0f, kRed),
                        text_segment(" +" + std::to_string(str), 17.0f, kWhite)
                    },
                    21.0f
                );
            }
            return;
        }

        if (type_ >= 10 && type_ <= 18) {
            const int level = type_ - 9;
            const int str = BowData::get_ability(type_, 0);
            const int agi = BowData::get_ability(type_, 1);
            const int fatal = BowData::get_ability(type_, 3);
            add_line({title_segment(std::string(kBowHurName) + std::to_string(level), 21.0f, kGreen)}, 25.0f);
            add_line({text_segment(kBowHurDesc, 17.0f, kDesc)}, 21.0f);
            Line stats = {
                text_segment(kStrength, 17.0f, kRed),
                text_segment(" +" + std::to_string(str) + ", ", 17.0f, kWhite),
                text_segment(kAgility, 17.0f, kGreen),
                text_segment(" +" + std::to_string(agi), 17.0f, kWhite)
            };
            if (fatal > 0) {
                stats.push_back(text_segment(", ", 17.0f, kWhite));
                stats.push_back(text_segment(kFatalBlow, 17.0f, kYellow));
                stats.push_back(text_segment(" +" + std::to_string(fatal), 17.0f, kWhite));
            }
            lines_.push_back(std::move(stats));
            line_heights_.push_back(21.0f);
            return;
        }

        if (type_ >= 19 && type_ <= 27) {
            const int level = type_ - 18;
            const int str = BowData::get_ability(type_, 0);
            const int agi = BowData::get_ability(type_, 1);
            const int power = BowData::get_ability(type_, 2);
            const int fatal = BowData::get_ability(type_, 3);
            const int multi = BowData::get_ability(type_, 4);
            add_line({title_segment(std::string(kBowPhaName) + std::to_string(level), 21.0f, kPhantom)}, 25.0f);
            add_line({text_segment(kBowPhaDesc, 17.0f, kDesc)}, 21.0f);

            if (power == 0) {
                add_line(
                    {
                        text_segment(kStrength, 17.0f, kRed),
                        text_segment(" +" + std::to_string(str) + ", ", 17.0f, kWhite),
                        text_segment(kAgility, 17.0f, kGreen),
                        text_segment(" +" + std::to_string(agi), 17.0f, kWhite),
                        text_segment(", ", 17.0f, kWhite),
                        text_segment(kFatalBlow, 17.0f, kYellow),
                        text_segment(" +" + std::to_string(fatal), 17.0f, kWhite)
                    },
                    21.0f
                );
            } else {
                add_line(
                    {
                        text_segment(kStrength, 17.0f, kRed),
                        text_segment(" +" + std::to_string(str) + ", ", 17.0f, kWhite),
                        text_segment(kAgility, 17.0f, kGreen),
                        text_segment(" +" + std::to_string(agi) + ", ", 17.0f, kWhite),
                        text_segment(kPowerShot, 17.0f, kYellow),
                        text_segment(" +" + std::to_string(power), 17.0f, kWhite)
                    },
                    21.0f
                );

                Line extra = {
                    text_segment(kFatalBlow, 17.0f, kYellow),
                    text_segment(" +" + std::to_string(fatal), 17.0f, kWhite)
                };
                if (multi > 0) {
                    extra.push_back(text_segment(" ,", 17.0f, kWhite));
                    extra.push_back(text_segment(kMultiArrow, 17.0f, kPhantom));
                    extra.push_back(text_segment(" +" + std::to_string(multi), 17.0f, kWhite));
                }
                lines_.push_back(std::move(extra));
                line_heights_.push_back(21.0f);
            }
            return;
        }

        const int temp1 = BowData::get_ability(28, 0);
        const int temp2 = BowData::get_ability(28, 1);
        const int temp3 = BowData::get_ability(28, 2);
        const int temp4 = BowData::get_ability(28, 3);
        const int temp5 = BowData::get_ability(28, 4);
        add_line({title_segment(kBowFfName, 21.0f, kFinal)}, 25.0f);
        add_line(
            {
                text_segment(kBowFfDesc, 17.0f, kDesc),
                text_segment(kBowFfDesc2, 17.0f, kYellow)
            },
            21.0f
        );
        add_line({text_segment(kBowAbilityCurrent, 17.0f, kYellow)}, 21.0f);
        add_line(
            {
                text_segment(kStrength, 17.0f, kRed),
                text_segment(" +" + std::to_string(temp1) + ", ", 17.0f, kWhite),
                text_segment(kAgility, 17.0f, kGreen),
                text_segment(" +" + std::to_string(temp2) + ", ", 17.0f, kWhite)
            },
            21.0f
        );
        add_line(
            {
                text_segment(kPowerShot, 17.0f, kYellow),
                text_segment(" +" + std::to_string(temp3) + ", ", 17.0f, kWhite),
                text_segment(kFatalBlow, 17.0f, kYellow),
                text_segment(" +" + std::to_string(temp4) + ", ", 17.0f, kWhite),
                text_segment(kMultiArrow, 17.0f, kPhantom),
                text_segment(" +" + std::to_string(temp5) + ".", 17.0f, kWhite)
            },
            21.0f
        );
        return;
    }

    const int add_level = zone_ != nullptr ? zone_->get_add_level(type_) : 0;
    switch (type_) {
    case 0:
        add_line({title_segment(kCityWall, 25.0f, kYellow)}, 29.0f);
        add_line({text_segment(kCityWallDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_), SkillData::get_value(type_, 1));
        break;
    case 1:
        add_line({title_segment(kLavaMoat, 25.0f, kRed)}, 29.0f);
        add_line({text_segment(kLavaMoatDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kLavaMoatUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 2:
        add_line({title_segment(kBurn, 25.0f, kYellow)}, 29.0f);
        add_line({text_segment(kBurnDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_), SkillData::get_value(type_, 1));
        break;
    case 3:
        add_line({title_segment(kEntanglingLava, 25.0f, kYellow)}, 29.0f);
        add_line({text_segment(kEntanglingLavaDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_), SkillData::get_value(type_, 1), true);
        break;
    case 4:
        add_line({title_segment(kMagicTower, 25.0f, kTowerIce)}, 29.0f);
        add_line({text_segment(kMagicTowerDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kMagicTowerUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 5:
        add_line({title_segment(kMagicPower, 25.0f, kYellow)}, 29.0f);
        add_line({text_segment(kMagicPowerDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_), SkillData::get_value(type_, 1));
        break;
    case 6:
        add_line({title_segment(kSplash, 25.0f, kRed)}, 29.0f);
        add_line({text_segment(kSplashDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_), SkillData::get_value(type_, 1), true);
        break;
    case 7:
        add_line({title_segment(kStrength, 25.0f, kRed)}, 29.0f);
        add_line({text_segment(kStrengthDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_, add_level), SkillData::get_value(type_, add_level + 1));
        break;
    case 8:
        add_line({title_segment(kAgility, 25.0f, kGreen)}, 29.0f);
        add_line({text_segment(kAgilityDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_, add_level), SkillData::get_value(type_, add_level + 1));
        break;
    case 9:
        add_line({title_segment(kPowerShot, 25.0f, kRed)}, 29.0f);
        add_line({text_segment(kPowerShotDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kPowerShotDesc2, 20.0f, kYellow)}, 24.0f);
        break;
    case 10:
        add_line({title_segment(kFatalBlow, 25.0f, kGreen)}, 29.0f);
        add_line({text_segment(kFatalBlowDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_, add_level), SkillData::get_value(type_, add_level + 1), true);
        break;
    case 11: {
        const int item_level = ItemParam::get_level(type_);
        int arrows_now = ((add_level + item_level) + 7) / 4;
        int arrows_next = ((add_level + item_level) + 8) / 4;
        arrows_now = std::min(arrows_now, 5);
        arrows_next = std::min(arrows_next, 5);
        add_line({title_segment(kMultiArrow, 21.0f, kBlue)}, 25.0f);
        add_line({text_segment(kMultiArrowDesc, 17.0f, kDesc)}, 21.0f);
        add_line(
            {
                text_segment(std::string(kCurrent) + " ", 17.0f, kYellow),
                text_segment(
                    std::to_string(arrows_now) + " " + kMultiArrowPlural + "  " +
                        std::to_string(SkillData::get_value(type_, add_level)) + "% " + kDamage,
                    17.0f,
                    kWhite
                )
            },
            21.0f
        );
        add_line(
            {
                text_segment(std::string(kNext) + " ", 17.0f, kYellow),
                text_segment(
                    std::to_string(arrows_next) + " " + kMultiArrowPlural + "  " +
                        std::to_string(SkillData::get_value(type_, add_level + 1)) + "% " + kDamage,
                    17.0f,
                    kWhite
                )
            },
            21.0f
        );
        break;
    }
    case 12:
        add_line({title_segment(kSeniorHunter, 25.0f, kMagenta)}, 29.0f);
        add_line({text_segment(kSeniorHunterDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_, add_level), SkillData::get_value(type_, add_level + 1), true);
        break;
    case 13:
        add_line({title_segment(kPoisonArrow, 25.0f, kGreen)}, 29.0f);
        add_line({text_segment(kPoisonArrowDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_, add_level), SkillData::get_value(type_, add_level + 1), true);
        break;
    case 14:
        add_line({title_segment(kManaResearch, 25.0f, kYellow)}, 29.0f);
        add_line({text_segment(kManaResearchDesc, 20.0f, kDesc)}, 24.0f);
        add_value_line(SkillData::get_value(type_), SkillData::get_value(type_, 1));
        break;
    case 15:
        add_line({title_segment(kFireBall, 25.0f, kRed)}, 29.0f);
        add_line({text_segment(kFireBallDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kFireUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 16:
        add_line({title_segment(kMeteor, 25.0f, kRed)}, 29.0f);
        add_line({text_segment(kMeteorDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kFireUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 17:
        add_line({title_segment(kArmageddon, 25.0f, kRed)}, 29.0f);
        add_line({text_segment(kArmageddonDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kFireUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 18:
        add_line({title_segment(kGlacialSpike, 25.0f, kTowerIce)}, 29.0f);
        add_line({text_segment(kGlacialSpikeDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kIceUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 19:
        add_line({title_segment(kFrostNova, 25.0f, kTowerIce)}, 29.0f);
        add_line({text_segment(kFrostNovaDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kIceUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 20:
        add_line({title_segment(kIceAge, 25.0f, kTowerIce)}, 29.0f);
        add_line({text_segment(kIceAgeDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kIceUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 21:
        add_line({title_segment(kLightningStrike, 25.0f, kLight)}, 29.0f);
        add_line({text_segment(kLightningStrikeDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kLightUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 22:
        add_line({title_segment(kThunderStorm, 25.0f, kLight)}, 29.0f);
        add_line({text_segment(kThunderStormDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kLightUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    case 23:
        add_line({title_segment(kRagnarok, 25.0f, kLight)}, 29.0f);
        add_line({text_segment(kRagnarokDesc, 20.0f, kDesc)}, 24.0f);
        add_line({text_segment(kLightUpgrade, 20.0f, kYellow)}, 24.0f);
        break;
    default:
        break;
    }
}

void Description::draw() {
    if (refresh_flag_) {
        refresh_text();
        refresh_flag_ = false;
    }

    auto& fonts = FontRenderer::instance();
    const auto& description_layout = research_layout_tweak(ResearchLayoutElement::DescriptionText);
    const float base_x = sx(kBaseX + description_layout.offset_x);
    const float base_y = sy(kBaseY + description_layout.offset_y);
    const float wrap_width = sx(kWrapWidth * description_layout.scale);
    struct WrappedLineEntry {
        Line line;
        float line_height = 0.0f;
    };

    std::vector<WrappedLineEntry> wrapped_entries;
    wrapped_entries.reserve(lines_.size());
    float total_logical_height = 0.0f;
    for (std::size_t line_index = 0; line_index < lines_.size(); ++line_index) {
        const auto& line = lines_[line_index];
        float fallback_line_height = 0.0f;
        for (const auto& segment : line) {
            fallback_line_height = std::max(fallback_line_height, segment.size);
        }
        const float line_height = line_heights_[line_index] > 0.0f ? line_heights_[line_index] : fallback_line_height;
        if (line.empty()) {
            wrapped_entries.push_back({{}, line_height});
            total_logical_height += line_height + (Param::language == 1 ? -1.0f : 0.0f);
            continue;
        }

        const auto wrapped_lines = wrap_line_segments(fonts, line, wrap_width);
        for (const auto& wrapped_line : wrapped_lines) {
            wrapped_entries.push_back({wrapped_line, line_height});
            total_logical_height += line_height + (Param::language == 1 ? -1.0f : 0.0f);
        }
    }

    const float available_height = std::max(1.0f, kWrapHeight * description_layout.scale);
    const float layout_scale = total_logical_height > available_height
        ? (available_height / total_logical_height)
        : 1.0f;

    float draw_y = base_y;
    for (const auto& entry : wrapped_entries) {
        if (entry.line.empty()) {
            draw_y += sy((entry.line_height + (Param::language == 1 ? -1.0f : 0.0f)) * layout_scale);
            continue;
        }
        draw_line_segments(fonts, entry.line, draw_y, base_x, layout_scale);
        draw_y += sy((entry.line_height + (Param::language == 1 ? -1.0f : 0.0f)) * layout_scale);
    }
}

} // namespace defender
