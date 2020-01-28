#ifndef __DISPATCHER_HPP_
#define __DISPATCHER_HPP_

#define DEFAULTHEARTTIMER 30

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
//#include <net/if.h>
#include <cstring>
#include <cstdio>
#include "packets.hpp"
#include "loguru.hpp"
#include "Tracker.hpp"
#include <ctime>
#include <thread>
#include <mutex>


#define MAXBUFSIZE 1024     // todo recheck according to img size


class Dispatcher final {
private:
    const unsigned short _port;
    int sockfd;
    sockaddr_in srvaddr;
    unsigned short _nodeId;
    Tracker *_tracker;
    std::thread _threadHeartBeat;
    unsigned short _heartTimer;

    void _setup_socket();
    void _hearbeat();
    
public:
    Dispatcher(unsigned short port, unsigned short nodeID, Tracker *tracker);
    Dispatcher(unsigned short port, unsigned short nodeID, Tracker *tracker, unsigned short heartTimer);
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher(Dispatcher&&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&&) = delete;
    ~Dispatcher();

    void start();
};

#endif
