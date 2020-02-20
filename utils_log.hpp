#ifndef _UTILS_LOG_HPP_
#define _UTILS_LOG_HPP_

#include <string>
#include <map>
#include <iostream>
#include "Position.hpp"

std::string print_log_map(const std::map<uint8_t, std::pair<uint32_t, unsigned int>>& map);

std::string print_log_map(const std::map<uint8_t, std::pair<Position, uint32_t>>& map);

#endif