#ifndef __IMAGECHUNKPACKET_HPP_
#define __IMAGECHUNKPACKET_HPP_

#include <array>
#include "Position.hpp"
#include "settings.hpp"


struct ImageChunkPacket  {  // Image identified by <nodeID, timestamp>
    uint8_t nodeID=0;
    uint32_t timestamp;
    Position position;
    uint32_t offset;
    uint32_t sizeImage;
    std::array<char,IMG_CHUNK_SIZE> chunk_content;
    

    ImageChunkPacket(uint8_t nodeID, uint32_t timestamp, uint32_t offset,  uint32_t sizeImage, std::array<char,IMG_CHUNK_SIZE> chunk_content) :
        nodeID(nodeID), timestamp(timestamp), position(), offset(offset), sizeImage(sizeImage), chunk_content(chunk_content) {}
    ImageChunkPacket() :
        nodeID(0), timestamp(0), position(), offset(0), sizeImage(0), chunk_content() {}

    std::string repr() const {
        const size_t len=256;
        char buffer[len];
        snprintf(buffer, len, "{nodeID=%d|offset=%d|timestamp=%d|Pos=(%f,%f)|sizeImage=%d}", nodeID, offset, timestamp, position.longitude, position.latitude, sizeImage);
        return buffer;
    }
};

#endif