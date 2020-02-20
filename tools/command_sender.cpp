#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <cstring>
#include <vector>
#include <thread>
#include "../loguru.hpp"
#include "../settings.hpp"
#include "../CCPackets.hpp"
#include "../CCCommands.hpp"
#include "../utils.hpp"


#define SRV_IP "127.0.0.1"
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


int main(int argc, char const *argv[]) 
{ 
    int sockfd = 0, valread; 
    struct sockaddr_in srv_addr; 
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    srv_addr.sin_family = AF_INET; 
    srv_addr.sin_port = htons(CC_SERVER_PORT); 
    srv_addr.sin_addr.s_addr = inet_addr(SRV_IP);
    memset(srv_addr.sin_zero, 0, sizeof(srv_addr.sin_zero));

    char buffer[4096];
    memset(buffer, '\0', 4096);
    uint32_t size_file_remaining;
    uint32_t bytes_to_treat(CC_IMAGE_CHUNK);

    if (connect(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    while (true) {
        FILE* image_file;
        uint8_t command;
        std::cout << "Input uint8_t command : ";
        command = input_uint8();
        send(sockfd , &command , sizeof(command), 0 );
        printf("sent command=%d\n", command);

        if (command == FETCH_NODES_POS) {
            if (recv(sockfd, &buffer, 4096, 0) < 0) {
                perror("Cannot recv the node map");
            }
            printf("String received : %s \n", buffer);

        }
        else if (command == NEW_IMAGE) {
            size_file_remaining = get_size(PATH_IMG_COMPLETE);
            image_file = fopen(PATH_IMG_COMPLETE, "rb");

            if (IMG_CHUNK_SIZE > size_file_remaining) {
                bytes_to_treat = size_file_remaining;
            }
            else {
                bytes_to_treat = IMG_CHUNK_SIZE;
            }

            while(size_file_remaining > 0 and !feof(image_file)) {
                fread(&buffer, sizeof(char), bytes_to_treat, image_file);  // TODO send

                size_file_remaining -= bytes_to_treat;

                memset(buffer, '\0', 4096);

                if (IMG_CHUNK_SIZE <= size_file_remaining) {
                    bytes_to_treat = IMG_CHUNK_SIZE;
                }
                else {
                    bytes_to_treat = size_file_remaining;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(IMAGE_SEND_SLEEP_SEPARATOR));
            }

            fclose(image_file);

        }
        memset(buffer, '\0', 4096);
    }

    return 0; 
} 