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
#include <fstream>

#define EXIT_PROG_CODE 99
#define SRVPORT 5821
#define LIMITUINT8 256  // 2**8
#define IMG_CHUNK_SIZE 250
#define SIZE_PATH 50

uint32_t get_size(char* path) {
    uint32_t res;

    std::ifstream in_file(path, std::ios::binary);
    in_file.seekg(0, std::ios::end);
    res = static_cast<uint32_t>(in_file.tellg());
    in_file.close();

    return res;
}


char* input_filename() {
    char *buffer = new char[SIZE_PATH];
    memset(buffer, '\0', SIZE_PATH);

    std::cout << "File-Path : ";
    if (std::cin.peek() != '\n') {
        std::cin >> buffer;
    }
    else {
        strcpy(buffer, "/home/heikko/CLionProjects/mnetR/img/test");
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return buffer;
}


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


ImageChunkPacket input_packet() {
    ImageChunkPacket packet;

    packet.nodeID = 200;
    packet.seqnum = 454;
    packet.timestamp = static_cast<uint32_t>(std::time(nullptr));
    packet.position = Position(0,0);

    std::cout << "nodeID : ";
    if (std::cin.peek() != '\n') {
        packet.nodeID = input_uint8();
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "seqnum : ";
    if (std::cin.peek() != '\n') {
        std::cin >> packet.seqnum;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Position : ";
    if (std::cin.peek() != '\n') {
        std::cout << "Longitude:";
        std::cin >> packet.position.longitude;
        std::cout << "Latitude:";
        std::cin >> packet.position.latitude;
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
    std::ifstream image;
    char *path;


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

#ifdef __aarch64__
    std::cout << "ARM architecture detected" << std::endl;
        memset(&b_iface, 0, sizeof(b_iface));
        snprintf(b_iface.ifr_name, sizeof(b_iface.ifr_name), b_iface_name);
        if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void*)&b_iface, sizeof(b_iface)) < 0) {
            close(sockfd);
            perror("setsockopt (SO_BINDTODEVICE)");
            throw;
        }
#else
    std::cout << "Standard architecture detected" << std::endl;
#endif

    // filling socket information for reception
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    srvaddr.sin_port = htons(SRVPORT);
    // networking - end

    while (! stop) {
        ImageChunkPacket packet = input_packet();
        memset(&packet.chunk_content, '\0', IMG_CHUNK_SIZE);
        path = input_filename();
        image.open(path, std::ios::binary);
        packet.sizeImage = get_size(path);

        while(image.read(&packet.chunk_content[0], IMG_CHUNK_SIZE)) {
            if ( sendto(sockfd, &packet, sizeof(ImageChunkPacket), 0, reinterpret_cast<const sockaddr*>(&srvaddr), len_srvaddr)  < 0 ) {
                close(sockfd);
                perror("Cannot sendto chunk");
                throw;
            }
            packet.seqnum += 1;
            packet.offset += IMG_CHUNK_SIZE;

            memset(&packet.chunk_content, '\0', IMG_CHUNK_SIZE);
            printf("ImageChunkPacket: %s\n", packet.repr().c_str());
        }

        image.close();
        delete(path);
        path = nullptr;
//        getchar();
    }

    close(sockfd);
    return 0;
}