#ifndef __IMAGE_HPP_
#define __IMAGE_HPP_


#include <iostream>
#include <vector>
#include <array>
#include "settings.hpp"


struct Image {
    uint8_t nodeID;
    uint32_t timestamp;
    std::vector<std::array<char, CHUNK_SIZE>> content;

    Image(uint8_t nodeID, uint32_t timestamp, Position pos, bool led_status) :
            nodeID(nodeID), timestamp(timestamp), position(pos), led_status(led_status) {}
    Image() :
            nodeID(0), seqnum(0), timestamp(0), position(0,0), led_status(false) {}


    std::string repr() const {
        const size_t len=256;
        char buffer[len];
        snprintf(buffer, len, "{nodeID=%d|seqnum=%d|timestamp=%d|Pos=(%f,%f), led_status=%d}", nodeID, seqnum, timestamp, position.longitude, position.latitude, led_status);
        return buffer;
    }
};


#endif
