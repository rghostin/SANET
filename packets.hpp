#ifndef __PACKETS_HPP__
#define __PACKETS_HPP__

#include <string>
#include "Position.hpp"

struct TrackPacket  {
    uint8_t nodeID=0;
    uint32_t seqnum=0;
    uint32_t timestamp;
    Position position;
    bool led_status=false;


    TrackPacket(uint8_t nodeID, uint32_t seqnum, uint32_t timestamp, Position pos, bool led_status) :
        nodeID(0), seqnum(0), timestamp(timestamp), position(pos), led_status(led_status) {}

    TrackPacket() :
        nodeID(0), seqnum(0), timestamp(0), position(), led_status(false) {}

    std::string repr() const {
        const size_t len=256;
        char buffer[len];
        snprintf(buffer, len, "{nodeID=%d|seqnum=%d|timestamp=%d|Pos=(%f,%f)}", nodeID, led_status, timestamp, seqnum, position.longitude, position.latitude);
        return buffer;
    }    
};


#endif