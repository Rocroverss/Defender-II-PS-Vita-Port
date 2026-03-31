#include "game/umeng_helper.hpp"

#include <algorithm>
#include <cctype>
#include <locale>
#include <string>

#include "game/save.hpp"

namespace defender {
namespace {

bool s_initialized = false;
bool s_session_open = false;
int s_last_level = 0;

std::string to_lower_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

} // namespace

void UMengHelper::init() {
    s_initialized = true;
}

void UMengHelper::on_resume() {
    if (s_initialized) {
        s_session_open = true;
    }
}

void UMengHelper::on_pause() {
    s_session_open = false;
}

std::string UMengHelper::get_ty_channel() {
    std::string locale_name = "en";
    try {
        locale_name = std::locale("").name();
    } catch (...) {
        locale_name = "en";
    }

    size_t split = locale_name.find_first_of("_.-");
    std::string language = (split == std::string::npos) ? locale_name : locale_name.substr(0, split);
    if (language.empty()) {
        language = "en";
    }
    return "TY_" + to_lower_ascii(language);
}

void UMengHelper::on_purchase(int, const std::string&, int) {
    if (!s_initialized) {
        return;
    }
    // Analytics SDK is Android-only; Vita port keeps a safe no-op stub.
    (void)Save::load_data("newbiePacktotalBuyAmount");
}

void UMengHelper::start_level(int level) {
    if (!s_initialized) {
        return;
    }
    s_last_level = level;
}

void UMengHelper::end_level(int level, bool) {
    if (!s_initialized) {
        return;
    }
    s_last_level = level;
}

} // namespace defender

