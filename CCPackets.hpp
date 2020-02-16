#ifndef _CCPACKETS_HPP
#define _CCPACKETS_HPP

#include <cstdint>

struct NodePositionPacket{
    uint8_t nodeID;
    double longitude;
    double latitude;
};

#endif
