#ifndef __PACKETS_HPP__
#define __PACKETS_HPP__

#include <string>


struct Packet {
    uint8_t nodeID;
    bool led_status=0;
    uint32_t timestamp;
};


inline void print_packet(const Packet& packet) {
    printf("nodeID : %d\nled_status: %d\ntimestamp: %d\n\n", packet.nodeID, packet.led_status, packet.timestamp);
}


inline std::string get_packetInfos(const Packet& packet) {
    return "nodeID : " + std::to_string(packet.nodeID) + " | led_status: " + std::to_string(packet.led_status) + " | timestamp: " + std::to_string(packet.timestamp);
}

#endif