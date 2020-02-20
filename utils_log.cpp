#include "utils_log.hpp"

std::string print_log_map(const std::map<uint8_t, std::pair<uint32_t, unsigned int>>& map) {
    std::string res;
    for (const auto & it : map) {
        res += "{" + std::to_string(static_cast<int>(it.first)) + ";" + std::to_string((it.second).first) + ";" + std::to_string((it.second).second) + "}\n";
    }
    return res;
}


std::string print_log_map(const std::map<uint8_t, std::pair<Position, uint32_t>>& map) {
    std::string res;
    for (const auto & it : map) {
        res += "{" + std::to_string(static_cast<int>(it.first)) + ";" + std::to_string((it.second).first.longitude) + ":" + std::to_string((it.second).first.latitude) + ";" + std::to_string((it.second).second) + "}\n";
    }
    return res;
}
