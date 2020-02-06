#ifndef _UTILS_LOG_HPP_
#define _UTILS_LOG_HPP_

#include <string>
#include <map>
#include <iostream>
#include "Position.hpp"

inline std::string print_log_map(const std::map<uint8_t, std::pair<uint32_t, unsigned int>>& map) {
    std::string res="";
    for (auto it=map.cbegin(); it!=map.cend(); ++it) {
        res += "{" + std::to_string(it->first) + ";" + std::to_string((it->second).first) + ";" + std::to_string((it->second).second) + "}\n";
    }
    return res;
}


inline std::string print_log_map(const std::map<uint8_t, std::pair<Position, uint32_t>>& map) {
    std::string res="";
    for (auto it=map.cbegin(); it!=map.cend(); ++it) {
        res += "{" + std::to_string(it->first) + ";" + std::to_string((it->second).first.longitude) + ":" + std::to_string((it->second).first.latitude) + ";" + std::to_string((it->second).second) + "}\n";
    }
    return res;
}

#endif