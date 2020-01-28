#ifndef __PACKETS_HPP__
#define __PACKETS_HPP__

struct Packet {
    uint8_t nodeID;
    bool led_status=0;
    uint32_t timestamp;
};

inline void print_packet(const Packet& packet) {
    printf("nodeID : %d\nled_status: %d\ntimestamp: %d\n\n", packet.nodeID, packet.led_status, packet.timestamp);
}

inline *char getPacketInfos(const Packet& packet){ // TODO supprimer toute dépendances à string
    std::string result;

    std::string str_nodeID = "nodeID : ";
    int var_nodeID = packet.nodeID ;
    std::string str_led_status = " \nled_status : ";
    bool var_led_status = packet.led_status;
    std::string str_timestamp = " \ntimestamp : ";
    uint32_t var_timestamp = packet.timestamp;
    std::string str_end = "\n\n";

    result = str_nodeID + std::to_string(var_nodeID) + str_led_status + std::to_string(var_led_status) + str_timestamp + std::to_string(var_timestamp);
    char* ch = strdup(result.c_str());

    return ch;
};
#endif