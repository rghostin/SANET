#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>
#include <ctime>
#include <limits>
#include "packets.hpp"

#define EXIT_PROG_CODE 99
#define SRVPORT 5820
#define LIMITUINT8 256  // 2**8


uint8_t input_uint8() {
    uint8_t uint8_x;
    char tmp_uint8_input[3];

    std::cin >> tmp_uint8_input;
    int x = std::atoi(tmp_uint8_input);

    if (x < 0 or LIMITUINT8 < x) {
        perror("Invalid input uint8_t");
        throw;
    }
    else {
        uint8_x = static_cast<uint8_t>(x);
    }

    return uint8_x;
}


TrackPacket input_packet() {
    TrackPacket packet;

    /*std::cout << "nodeID : ";
    packet.nodeID = input_uint8();
    std::cout << "led_status : ";
    std::cin >> packet.led_status;
    std::cout << "Timestamp : ";
    std::cin >> packet.timestamp;
    std::cout << "seqnum : ";
    std::cin >> packet.seqnum;*/

    packet.nodeID = 200;
    packet.seqnum = 454;
    packet.timestamp = std::time(nullptr);
    packet.position = Position(0,0);
    packet.led_status = false;

    std::cout << "nodeID : ";
    if (std::cin.peek() != '\n') {
        packet.nodeID = input_uint8();
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "seqnum : ";
    if (std::cin.peek() != '\n') {
        std::cin >> packet.seqnum;
    }

    std::cout << "Position : ";
    if (std::cin.peek() != '\n') {
        std::cout << "Longitude:";
        std::cin >> packet.position.longitude;
        std::cout << "Latitude:";
        std::cin >> packet.position.latitude;
    }

    std::cout << "led_status : ";
    if (std::cin.peek() != '\n') {
        std::cin >> packet.led_status;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return packet;
}


int main(int argc, char** argv) {
    int sockfd;
    int broadcast_flag = 1;
    sockaddr_in srvaddr;
    socklen_t len_srvaddr = sizeof(sockaddr);
    bool stop=false;

    char input_buffer[1024];
    uint8_t packet_type;
    
    // networking - start
    // create socket
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Cannot create socket");
        throw;
    }

    // Enable broadcast
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_flag, sizeof(broadcast_flag)) < 0) {
        close(sockfd);
        perror("setsockopt (SO_BROADCAST)");
        throw;
    }

    // TODO setting interface for bat0
    //

    // filling socket information for reception
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); // inet_addr("164.15.121.78");
    srvaddr.sin_port = htons(SRVPORT);
    // networking - end


    while (! stop) {
        TrackPacket packet = input_packet();
        if ( sendto(sockfd, &packet, sizeof(TrackPacket), 0, reinterpret_cast<const sockaddr*>(&srvaddr), len_srvaddr)  < 0 ) {
            close(sockfd);
            perror("Cannot sendto");
            throw;
        }

        printf("TrackPacket: %s\n", packet.repr().c_str());
        getchar();
    }

    close(sockfd);
    return 0;
}
