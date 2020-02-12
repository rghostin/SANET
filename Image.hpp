#ifndef __IMAGE_HPP_
#define __IMAGE_HPP_

#include <cstdint>
#include <vector>
#include "settings.hpp"
#include "Position.hpp"


struct Image {
    uint8_t nodeID;
    uint32_t timestamp;
    Position position;
    std::vector<char> content;

    Image(uint8_t nodeID, uint32_t timestamp, Position pos, std::vector<char> content) :
            nodeID(nodeID), timestamp(timestamp), position(pos), content(std::move(content)) {}
    Image() :
            nodeID(0), timestamp(0), position(0,0), content() {}

    std::pair<uint8_t, uint32_t> get_imageID() {
        return std::make_pair(nodeID, timestamp);
    }
};

#endif
