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
#include "../ImageChunkPacket.hpp"
#include <fstream>
#include <thread>
#include <chrono>
#include <openssl/md5.h>
#include "../settings.hpp"

#define EXIT_PROG_CODE 99
#define SRVPORT 5821
#define LIMITUINT8 256  // 2**8
#define SIZE_PATH 50

uint32_t get_size(char* path) {
    uint32_t res;

    std::ifstream in_file(path, std::ios::binary);
    in_file.seekg(0, std::ios::end);
    res = static_cast<uint32_t>(in_file.tellg());
    in_file.clear();
    in_file.seekg(0, std::ios::beg);


    unsigned char* md = new unsigned char[MD5_DIGEST_LENGTH];
    char *buffer = new char[res];
    memset(buffer, '\0', res);
    in_file.readsome(buffer, res);

    MD5(reinterpret_cast<unsigned char *>(buffer), res, md);

    int i;
    printf("Checksum MD5 : ");
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
        printf("%02x",md[i]);
    }
    printf("\n");

    in_file.close();
    delete[](md);
    delete[](buffer);

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
        strcpy(buffer, "img/imglink.txt");
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
    packet.timestamp = static_cast<uint32_t>(std::time(nullptr));
    packet.position = Position(0,0);

    std::cout << "nodeID : ";
    if (std::cin.peek() != '\n') {
        packet.nodeID = input_uint8();
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
    FILE* image_file;
    char *path;
    uint32_t size_file_remaining;
    uint32_t bytes_to_treat(IMG_CHUNK_SIZE);


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
        packet.sizeImage = get_size(path);
        size_file_remaining = packet.sizeImage;
        image_file = fopen(path, "rb");


        if (IMG_CHUNK_SIZE > size_file_remaining) {
            bytes_to_treat = size_file_remaining;
        }
        else {
            bytes_to_treat = IMG_CHUNK_SIZE;
        }

        while(size_file_remaining > 0 and !feof(image_file)) {
            fread(&packet.chunk_content[0], sizeof(char), bytes_to_treat, image_file);
            size_file_remaining -= bytes_to_treat;

            if ( sendto(sockfd, &packet, sizeof(ImageChunkPacket), 0, reinterpret_cast<const sockaddr*>(&srvaddr), len_srvaddr)  < 0 ) {
                close(sockfd);
                perror("Cannot sendto chunk");
                throw;
            }
            printf("ImageChunkPacket: %s\n", packet.repr().c_str());

            packet.offset += bytes_to_treat;

            memset(&packet.chunk_content, '\0', IMG_CHUNK_SIZE);

            if (IMG_CHUNK_SIZE <= size_file_remaining) {
                bytes_to_treat = IMG_CHUNK_SIZE;
            }
            else {
                bytes_to_treat = size_file_remaining;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(IMAGE_SEND_SLEEP_SEPARATOR));
        }
        printf("ImageChunkPacket: %s\n", packet.repr().c_str());

        fclose(image_file);

        delete(path);
        path = nullptr;
//        getchar();
    }

    close(sockfd);
    return 0;
}