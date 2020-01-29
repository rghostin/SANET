#ifndef __PACKETS_HPP__
#define __PACKETS_HPP__

#define SIZEBUFFER 256
#define SIZEARRAYCHARCONVERSION 30  // Utilisé pour timestamp

#include <charconv>
#include <array>


struct Packet {
    uint8_t nodeID;
    bool led_status=0;
    uint32_t timestamp;
};

inline void print_packet(const Packet& packet) {
    printf("nodeID : %d\nled_status: %d\ntimestamp: %d\n\n", packet.nodeID, packet.led_status, packet.timestamp);
}

inline char* getPacketInfos(const Packet& packet) {  // TODO DELETE du tableau crée à faire après !
    char myArray[] = "nodeID : X\nled_status: X\ntimestamp: X\n\n";
    char *buffer = new char[SIZEBUFFER];
    int sizeOfArray = sizeof(myArray) / sizeof(myArray[0]);

    int j(0); int compteurX(0);

    std::array<char, SIZEARRAYCHARCONVERSION> str;
    int sizeOfArray2 = sizeof(str) / sizeof(str[0]);


    // MEMSET MANUELLA
    for (int l = 0; l < SIZEBUFFER; ++l) { // MEMSET Buffer
        buffer[l] = '\000';
    }
    for (int k = 0; k < sizeOfArray2; ++k) { // MEMSET array of char used for conversion (timestamp + nodeID)
        str[k] = '\000';
    }

    std::to_chars(str.data(), str.data() + str.size(), packet.nodeID);  // On convertit déjà le NodeID en CHAR


    for (int i = 0; i < sizeOfArray; ++i) {
        if (myArray[i] == 'X') {
            switch (compteurX) {
                case 0:
                    for (int k = 0; k < sizeOfArray2; ++k) {
                        if (str[k] == '\000') {
                            break;
                        }
                        buffer[j] = str[k];
                        j += 1;
                    }
                    break;
                case 1:
                    if (packet.timestamp) {
                        buffer[j] = 'T';
                        buffer[j+1] = 'r';
                        buffer[j+2] = 'u';
                        buffer[j+3] = 'e';
                        j += 4;
                    }
                    else {
                        buffer[j] = 'F';
                        buffer[j+1] = 'a';
                        buffer[j+2] = 'l';
                        buffer[j+3] = 's';
                        buffer[j+4] = 'e';
                        j += 5;
                    }
                    break;
                case 2:
                    std::to_chars(str.data(), str.data() + str.size(), packet.timestamp);
                    sizeOfArray2 = sizeof(str) / sizeof(str[0]);
                    for (int k = 0; k < sizeOfArray2; ++k) {
                        buffer[j] = str[k];
                        j += 1;
                    }
                    break;
            }
            compteurX += 1;
        }
        else {
            buffer[j] = myArray[i];
            j += 1;
        }
    }

    return buffer;
}

#endif