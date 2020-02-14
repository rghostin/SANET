#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <map>
#include <iostream>
#include <string>




#define POS_FMT "Pos(%f,%f)"
#define POS_REPR(p) (p).longitude, (p).latitude

struct Position {
    double longitude;
    double latitude;

    Position() : longitude(0), latitude(0) {}
    Position(double longitude, double latitude) : longitude(longitude), latitude(latitude) {}
};

typedef std::map<uint8_t, std::pair<Position, uint32_t>> nodemap_t;


char* socket_path = "./usocket";

std::string get_encoded_json(const nodemap_t& map) {
    char buffer[4096]="";
    std::string res = "{";
    
    uint8_t _nodeid = 25;
    Position _mypos =  Position(12,24.2);



    for (auto it=map.cbegin(); it!=map.cend(); ++it) {
        uint32_t nodeid = it->first;
        Position pos = std::get<0>(it->second);

        snprintf(buffer, sizeof(buffer), "\"%u\": [%f, %f]", nodeid, pos.longitude, pos.latitude);
        res += buffer;
        res += ",";
        memset(buffer, 0, sizeof(buffer));
    }
    snprintf(buffer, sizeof(buffer), "\"%u\": [%f, %f]", _nodeid, _mypos.longitude, _mypos.latitude);
    res += buffer;
    res += "}";
    return res;
}

int main() {
    /*struct sockaddr_un addr;
    char buf[100];
    int usockfd,rc;


    if ( (usockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (*socket_path == '\0') {
        *addr.sun_path = '\0';
        strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
    } else {
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
    }

    if (connect(usockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        exit(-1);
    }*/

    std::map<uint8_t, std::pair<Position, uint32_t>> nodemap;

    /*nodemap[2] = {Position(2,3), 12345645};
    nodemap[5] = {Position(5,6), 65435435};
    nodemap[12] = { Position(5.24564, 4565.5757), 56535436 } ;*/

    std::string json_nodemap = get_encoded_json(nodemap);
    //std::string json_nodemap = "\{\"1\": \"2\"\}";
    std::cout << json_nodemap << std::endl;

    /*if (send(usockfd, json_nodemap.c_str(), json_nodemap.length(), 0) < 0) {
        perror("cannot send");
    }
    std::cout << "sent" << std::endl;*/

    return 0;
}
