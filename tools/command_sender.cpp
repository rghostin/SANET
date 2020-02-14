#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <cstring> 
#include "../loguru.hpp"
#include "../settings.hpp"


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
   
    if (connect(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    while (true) {
        uint8_t command;
        std::cout << "Input uint8_t command : ";
        command = input_uint8();
        send(sockfd , &command , sizeof(command), 0 );
        printf("sent command=%d\n", command);
    }

    return 0; 
} 