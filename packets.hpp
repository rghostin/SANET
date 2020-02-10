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


struct ImageChunkPacket  {  // Identified by <nodeID, timestamp>
    uint8_t nodeID=0;
    uint32_t seqnum=0;
    char* chunk_content;
    unsigned int offset;
    uint32_t timestamp;
    unsigned int sizeImage;


    ImageChunkPacket(uint8_t nodeID, uint32_t seqnum, char timestamp22[CHUNK_SIZE], unsigned int offset, uint32_t timestamp, unsigned int sizeImage) :
        nodeID(nodeID), seqnum(seqnum), chunk_content(timestamp22), offset(offset), timestamp(timestamp), sizeImage(sizeImage) {}
    ImageChunkPacket() :
        nodeID(0), seqnum(0), chunk_content(), offset(0), timestamp(0), sizeImage(0) {}


    std::string repr() const {
        const size_t len=256;
        char buffer[len];
        snprintf(buffer, len, "{nodeID=%d|seqnum=%d|offset=%d|timestamp=%d|sizeImage=%d}", nodeID, seqnum, offset, timestamp, sizeImage);
        return buffer;
    }
};


#endif