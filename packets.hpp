#ifndef __PACKETS_HPP__
#define __PACKETS_HPP__

#define PACKET_FMT "{nodeID=%d | led_status=%d | timestamp=%d}"
#define PACKET_REPR(p) (p).nodeID, (p).led_status, (p).timestamp

struct Packet {
    uint8_t nodeID;
    bool led_status=0;
    uint32_t timestamp;

    Packet() : nodeID(0), led_status(false), timestamp(0) {}
    Packet(uint8_t nodeID, bool led_status, uint32_t timestamp) : nodeID(nodeID), led_status(led_status), timestamp(timestamp) {}
};


#endif