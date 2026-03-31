#include "game/save.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

#include "game/achv_data.hpp"
#include "game/param.hpp"

namespace defender {
namespace {

struct SaveStore {
    std::unordered_map<std::string, int> ints;
    std::unordered_map<std::string, int64_t> times;
    std::unordered_map<std::string, std::string> strings;
};

std::array<SaveStore, 4> s_saves;
std::unordered_map<std::string, int> s_default_values;
bool s_inited = false;

std::string save_file_path(int file_num) {
#ifdef __vita__
    return "ux0:data/save" + std::to_string(file_num) + ".dat";
#else
    return "save" + std::to_string(file_num) + ".dat";
#endif
}

std::string hex_encode(const std::string& text) {
    static constexpr char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(text.size() * 2);
    for (unsigned char c : text) {
        out.push_back(kHex[(c >> 4) & 0x0F]);
        out.push_back(kHex[c & 0x0F]);
    }
    return out;
}

std::string hex_decode(const std::string& text) {
    auto hex_value = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };

    if ((text.size() % 2U) != 0U) {
        return {};
    }
    std::string out;
    out.reserve(text.size() / 2U);
    for (size_t i = 0; i < text.size(); i += 2U) {
        const int hi = hex_value(text[i]);
        const int lo = hex_value(text[i + 1U]);
        if (hi < 0 || lo < 0) {
            return {};
        }
        out.push_back(static_cast<char>((hi << 4) | lo));
    }
    return out;
}

std::string checksum_for_value(int value, const std::string& device_id) {
    // Java uses MD5(value + android_id); this is a deterministic lightweight parity stub.
    const std::string key = std::to_string(value) + device_id;
    uint64_t hash = 1469598103934665603ULL;
    for (unsigned char c : key) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    std::ostringstream oss;
    oss << std::hex << hash;
    return oss.str();
}

void flush_file(int file_num) {
    if (file_num < 0 || file_num >= static_cast<int>(s_saves.size())) {
        return;
    }
    std::ofstream out(save_file_path(file_num), std::ios::trunc);
    if (!out.is_open()) {
        return;
    }

    const SaveStore& store = s_saves[static_cast<size_t>(file_num)];
    for (const auto& kv : store.ints) {
        out << "I\t" << kv.first << "\t" << kv.second << "\n";
    }
    for (const auto& kv : store.times) {
        out << "L\t" << kv.first << "\t" << kv.second << "\n";
    }
    for (const auto& kv : store.strings) {
        out << "S\t" << kv.first << "\t" << hex_encode(kv.second) << "\n";
    }
}

void load_file(int file_num) {
    if (file_num < 0 || file_num >= static_cast<int>(s_saves.size())) {
        return;
    }
    SaveStore& store = s_saves[static_cast<size_t>(file_num)];
    store.ints.clear();
    store.times.clear();
    store.strings.clear();

    std::ifstream in(save_file_path(file_num));
    if (!in.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        const size_t p1 = line.find('\t');
        if (p1 == std::string::npos) continue;
        const size_t p2 = line.find('\t', p1 + 1U);
        if (p2 == std::string::npos) continue;

        const std::string type = line.substr(0, p1);
        const std::string key = line.substr(p1 + 1U, p2 - p1 - 1U);
        const std::string value = line.substr(p2 + 1U);

        try {
            if (type == "I") {
                store.ints[key] = std::stoi(value);
            } else if (type == "L") {
                store.times[key] = static_cast<int64_t>(std::stoll(value));
            } else if (type == "S") {
                store.strings[key] = hex_decode(value);
            }
        } catch (...) {
            // Skip malformed entry.
        }
    }
}

void ensure_initialized() {
    if (s_inited) {
        return;
    }
    (void)Save::get_default_value("");
    for (int i = 0; i < static_cast<int>(s_saves.size()); ++i) {
        load_file(i);
    }
    s_inited = true;
}

void encrypt_defaults() {
    const std::string device_id = Save::load_device_id();
    SaveStore& global = s_saves[Save::GLOBAL_DATA];
    global.strings[std::string(Save::CHECK_VALUE) + Save::GOLD] = checksum_for_value(100, device_id);
    global.strings[std::string(Save::CHECK_VALUE) + Save::STONE] = checksum_for_value(5, device_id);
    flush_file(Save::GLOBAL_DATA);
}

} // namespace

void Save::init() {
    ensure_initialized();
}

void Save::save_data(const std::string& type, int value) {
    save_data(type, value, GLOBAL_DATA);
}

void Save::pause_save_data() {
    save_data(GOLD, Param::gold);
    save_data(STONE, Param::stone);
    AchvData::save_achv_data();
}

void Save::save_time(const std::string& type, int64_t value) {
    ensure_initialized();
    SaveStore& global = s_saves[GLOBAL_DATA];
    global.times[type] = value;
    flush_file(GLOBAL_DATA);
}

int64_t Save::load_time(const std::string& type) {
    ensure_initialized();
    const int default_value = get_default_value(type);
    const SaveStore& global = s_saves[GLOBAL_DATA];
    const auto it = global.times.find(type);
    return it == global.times.end() ? static_cast<int64_t>(default_value) : it->second;
}

void Save::save_data(const std::string& type, int value, int file_num) {
    ensure_initialized();
    if (file_num < 0 || file_num >= static_cast<int>(s_saves.size())) {
        return;
    }

    SaveStore& store = s_saves[static_cast<size_t>(file_num)];
    store.ints[type] = value;
    if (type == GOLD || type == STONE) {
        const std::string device_id = load_device_id();
        store.strings[std::string(CHECK_VALUE) + type] = checksum_for_value(value, device_id);
    }
    flush_file(file_num);
}

void Save::clear_data(int file_num) {
    ensure_initialized();
    if (file_num < 0 || file_num >= static_cast<int>(s_saves.size())) {
        return;
    }
    s_saves[static_cast<size_t>(file_num)] = SaveStore{};
    flush_file(file_num);
}

void Save::clear_data() {
    clear_data(GLOBAL_DATA);
}

int Save::load_data(const std::string& type) {
    return load_data(type, GLOBAL_DATA);
}

std::string Save::load_device_id() {
    ensure_initialized();
    if (load_data(CHECK_DEVICEID) == 0) {
        auto now = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
        std::mt19937_64 rng(now ^ reinterpret_cast<uintptr_t>(&now));
        std::ostringstream id;
        id << "vita-" << std::hex << rng();
        s_saves[GLOBAL_DATA].strings[DEVICEID] = id.str();
        flush_file(GLOBAL_DATA);
        save_data(CHECK_DEVICEID, 1);
        encrypt_defaults();
    }

    const auto it = s_saves[GLOBAL_DATA].strings.find(DEVICEID);
    if (it != s_saves[GLOBAL_DATA].strings.end()) {
        return it->second;
    }
    return "vita-device";
}

std::string Save::load_name() {
    ensure_initialized();
    const auto it = s_saves[GLOBAL_DATA].strings.find(NAME);
    return (it == s_saves[GLOBAL_DATA].strings.end()) ? "player" : it->second;
}

void Save::save_name(const std::string& name) {
    ensure_initialized();
    s_saves[GLOBAL_DATA].strings[NAME] = name;
    flush_file(GLOBAL_DATA);
}

int Save::load_data(const std::string& type, int file_num) {
    ensure_initialized();
    if (file_num < 0 || file_num >= static_cast<int>(s_saves.size())) {
        return get_default_value(type);
    }

    const SaveStore& store = s_saves[static_cast<size_t>(file_num)];
    const auto it = store.ints.find(type);
    if (it != store.ints.end()) {
        return it->second;
    }
    return get_default_value(type);
}

int Save::get_default_value(const std::string& key) {
    ensure_default_values();
    const auto it = s_default_values.find(key);
    return (it == s_default_values.end()) ? 0 : it->second;
}

void Save::add_value(const std::string& key, int value) {
    s_default_values[key] = value;
}

void Save::new_game_init() {
    add_value(STAGE, 1);
    add_value(WIN, 0);
    add_value(LOSE, 0);
    add_value(LEVEL, 1);
    add_value(XP, 0);
    add_value(GOLD, 100);
    add_value(STONE, 5);
    add_value(BTL_LEVEL, 1);
    for (int i = 0; i < 10; ++i) {
        add_value(std::string(BOW_GET) + std::to_string(i), 1);
    }
    add_value(EQUIPED_BOW, 0);
    add_value("equipMagic0", 0);
    add_value("equipMagic1", -1);
    add_value("equipMagic2", -1);
    add_value(KILLS, 0);
    add_value(COST_COIN, 0);
    add_value(COST_STONE, 0);
    add_value(FIRE_CAST, 0);
    add_value(ICE_CAST, 0);
    add_value(LIGHT_CAST, 0);
    add_value("skillLevel7", 1);
    add_value("skillLevel8", 1);
    add_value("skillLevel0", 1);
    add_value("skillLevel14", 1);
    add_value("skillLevel15", 1);
}

void Save::ensure_default_values() {
    if (!s_default_values.empty()) {
        return;
    }
    new_game_init();
}

} // namespace defender
