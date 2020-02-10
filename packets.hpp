#ifndef __PACKETS_HPP__
#define __PACKETS_HPP__

#include <string>
#include "Position.hpp"
#include "settings.hpp"


struct TrackPacket  {
    uint8_t nodeID=0;
    uint32_t seqnum=0;
    uint32_t timestamp;
    Position position;
    bool led_status=false;


    TrackPacket(uint8_t nodeID, uint32_t seqnum, uint32_t timestamp, Position pos, bool led_status) :
        nodeID(nodeID), seqnum(seqnum), timestamp(timestamp), position(pos), led_status(led_status) {}
    TrackPacket() :
        nodeID(0), seqnum(0), timestamp(0), position(0,0), led_status(false) {}
    

    std::string repr() const {
        const size_t len=256;
        char buffer[len];
        snprintf(buffer, len, "{nodeID=%d|seqnum=%d|timestamp=%d|Pos=(%f,%f), led_status=%d}", nodeID, seqnum, timestamp, position.longitude, position.latitude, led_status);
        return buffer;
    }    
};


struct ImageChunkPacket  {  // Image identified by <nodeID, timestamp>
    uint8_t nodeID=0;
    uint32_t seqnum=0;
    uint32_t timestamp;
    uint32_t offset;
    uint32_t sizeImage;
    std::array<char,CHUNK_SIZE> chunk_content;
    

    ImageChunkPacket(uint8_t nodeID, uint32_t seqnum, uint32_t timestamp, uint32_t offset,  uint32_t sizeImage, std::array<char,CHUNK_SIZE> chunk_content) :
        nodeID(nodeID), seqnum(seqnum), timestamp(timestamp), offset(offset), sizeImage(sizeImage), chunk_content(chunk_content) {}
    ImageChunkPacket() :
        nodeID(0), seqnum(0), timestamp(0), offset(0), sizeImage(0), chunk_content() {}

    std::string repr() const {
        const size_t len=256;
        char buffer[len];
        snprintf(buffer, len, "{nodeID=%d|seqnum=%d|offset=%d|timestamp=%d|sizeImage=%d}", nodeID, seqnum, offset, timestamp, sizeImage);
        return buffer;
    }
};


#endif