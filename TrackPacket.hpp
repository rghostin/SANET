#ifndef __TRACKPACKET_HPP__
#define __TRACKPACKET_HPP__

#include <cstdint>
#include <array>
#include "settings.hpp"
#include "Position.hpp"

struct TrackPacket  {
    uint8_t nodeID=0;
    uint32_t seqnum=0;
    uint32_t timestamp;
    Position position;
    bool led_status=false;
    uint16_t polyid;
    std::array<char, TRACKING_GLOBALPOLY_MAXBUF> globalpoly;
    uint8_t mapid;

    TrackPacket(uint8_t nodeID, uint32_t seqnum, uint32_t timestamp, Position pos, bool led_status, uint16_t polyid, std::array<char, TRACKING_GLOBALPOLY_MAXBUF> globalpoly) :
        nodeID(nodeID), seqnum(seqnum), timestamp(timestamp), position(pos), led_status(led_status), polyid(polyid), globalpoly(globalpoly), mapid() {}
    TrackPacket() :
        nodeID(0), seqnum(0), timestamp(0), position(0,0), led_status(false), polyid(), globalpoly(), mapid() {}
    

    std::string repr() const {
        const size_t len=256;
        char buffer[len];
        snprintf(buffer, len, "{nodeID=%d|seqnum=%d|timestamp=%d|Pos=(%f,%f)|led_status=%d|poly=%s", nodeID, seqnum, timestamp, position.longitude, position.latitude, led_status, globalpoly.data());
        return buffer;
    }    
};

#endif