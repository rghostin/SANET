#ifndef __PACKETS_HPP__
#define __PACKETS_HPP__

#define PACKET_FMT "{nodeID=%d | led_status=%d | timestamp=%d | seqnum=%d}"
#define PACKET_REPR(p) (p).nodeID, (p).led_status, (p).timestamp, (p).seqnum

struct Packet {
    uint8_t nodeID;
    bool led_status=0;
    uint32_t timestamp;
    uint32_t seqnum=0;

    Packet() : nodeID(0), led_status(false), timestamp(0), seqnum(0) {}
    Packet(uint8_t nodeID, bool led_status, uint32_t timestamp, uint32_t seqnum) :
        nodeID(nodeID), led_status(led_status), timestamp(timestamp), seqnum(seqnum) {}
};


#endif