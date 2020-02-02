#ifndef __DISPATCHER_HPP_
#define __DISPATCHER_HPP_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <thread>
#include <mutex>
#include "loguru.hpp"
#include "common.hpp"
#include "packets.hpp"
#include "Tracker.hpp"


class MainServer {
private:
    // self information and settings
    const uint8_t _nodeID;
    unsigned short _heart_period;

    // private connection vars
    const unsigned short _port;
    sockaddr_in _srvaddr;

    // delegations
    Tracker& _tracker;

    // threads
    std::thread _thread_receiver;
    std::thread _thread_heartbeat;

    // next sequence for packet
    std::mutex _mutex_next_seqnum;
    uint32_t _next_seqnum=0;

    void _setup_socket_bind();
    Packet _produce_packet(bool led_status=false);
    Position _produce_position();
    void _tr_hearbeat();
    void _tr_receiver();

protected:
    // protected connection vars
    int sockfd;
    sockaddr_in _bc_sockaddr;

    virtual bool _to_be_ignored(const Packet&) const;
    virtual void _process_packet(const Packet&);
    
public:
    MainServer(unsigned short port, uint8_t nodeID, Tracker &tracker, unsigned short heartTimer);
    MainServer(const MainServer&) = delete;
    MainServer(MainServer&&) = delete;
    MainServer& operator=(const MainServer&) = delete;
    MainServer& operator=(const MainServer&&) = delete;
    virtual ~MainServer();

    virtual void start();
    virtual void join();
};

#endif
