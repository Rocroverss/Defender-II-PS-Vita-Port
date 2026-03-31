#pragma once

#include <string>

namespace defender {

class UMengHelper {
public:
    static constexpr const char* EVENT_PURCHASE = "purchase";
    static constexpr const char* EVENT_TOTAL_BUY_TIMES = "RepurchaseTimes";
    static constexpr int PAY_CHANNEL_GOOGLE = 21;

    static void init();
    static void on_resume();
    static void on_pause();
    static std::string get_ty_channel();

    static void on_purchase(int price, const std::string& product_id, int chip);
    static void start_level(int level);
    static void end_level(int level, bool is_success);
};

} // namespace defender

