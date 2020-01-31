#ifndef __DISPATCHER_HPP_
#define __DISPATCHER_HPP_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
//#include <net/if.h>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <thread>
#include <mutex>
#include "loguru.hpp"
#include "packets.hpp"
#include "Tracker.hpp"

#define MAXBUFSIZE 1024     // todo recheck according to img size


class MainServer {
private:
    std::mutex _mutex_next_seqnum;
    uint32_t _next_seqnum=0;

protected:
    // self information and settings
    const uint8_t _nodeID;
    unsigned short _heart_period;

    // connection
    const unsigned short _port;
    int sockfd;
    sockaddr_in _srvaddr;
    sockaddr_in _bc_sockaddr;

    std::thread _thread_receiver;

    // delegation 
    Tracker& _tracker;
    
    void _setup_socket_bind();

    // packet methods
    Packet _produce_packet(bool led_status=false);
    virtual bool _to_be_ignored(const Packet&) const;
    virtual void _process_packet(const Packet&) const;

    void _hearbeat();
    virtual void _receiver();
    
public:
    MainServer(unsigned short port, uint8_t nodeID, Tracker &tracker, unsigned short heartTimer);
    MainServer(const MainServer&) = delete;
    MainServer(MainServer&&) = delete;
    MainServer& operator=(const MainServer&) = delete;
    MainServer& operator=(const MainServer&&) = delete;
    virtual ~MainServer();

    virtual void start();
};

#endif
