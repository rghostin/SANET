#ifndef _ABSTRACTBROADCASTNODE_HPP_
#define _ABSTRACTBROADCASTNODE_HPP_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <thread>
#include <mutex>
#include "loguru.hpp"
#include "common.hpp"


template <typename P>
class AbstractBroadcastNode {
private:
    // private connection vars
    const unsigned short _port;
    int sockfd;
    sockaddr_in _srvaddr;
    sockaddr_in _bc_sockaddr;
    ifreq b_iface;
    const char* b_iface_name=BATMAN_IFACE;

    // threads
    std::thread _thread_receiver;
    void _tr_receiver();

    void _setup_socket_bind();

protected:
    const uint8_t _nodeID;
    const char* _name;
    virtual std::string threadname(std::string) const;

    virtual bool _to_be_ignored(const P&) const;
    virtual void _process_packet(const P&) = 0;
    virtual P _produce_packet();

public:
    AbstractBroadcastNode(uint8_t nodeID, unsigned short port, const char* name);
    AbstractBroadcastNode(const AbstractBroadcastNode&) = delete;
    AbstractBroadcastNode(AbstractBroadcastNode&&) = delete;
    AbstractBroadcastNode& operator=(const AbstractBroadcastNode&) = delete;
    AbstractBroadcastNode& operator=(const AbstractBroadcastNode&&) = delete;
    virtual ~AbstractBroadcastNode();

    virtual void start();
    virtual void join();
    virtual void broadcast(const P&) const final;
};



template<typename P>
AbstractBroadcastNode<P>::AbstractBroadcastNode(uint8_t nodeID, unsigned short port, const char* name):
    _port(port), _nodeID(nodeID), _name(name)
{
    // setup reception sockaddr
    memset(&_srvaddr, 0, sizeof(_srvaddr));
    _srvaddr.sin_family = AF_INET;
    _srvaddr.sin_addr.s_addr = INADDR_ANY;
    _srvaddr.sin_port = htons(_port);

    // setup broadcast destination sockaddr
    memset(&_bc_sockaddr, 0, sizeof(_bc_sockaddr));
    _bc_sockaddr.sin_family = AF_INET;
    _bc_sockaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    _bc_sockaddr.sin_port = htons(_port);
}


template<typename P>
AbstractBroadcastNode<P>::~AbstractBroadcastNode() {
    if (_thread_receiver.joinable()) {
        _thread_receiver.join();
    }
    close(sockfd);
}


template<typename P>
void AbstractBroadcastNode<P>::_setup_socket_bind() {
    // create socket
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Cannot create socket");
        throw;
    }

    int broadcast_flag = 1;
    // Enable broadcast
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_flag, sizeof(broadcast_flag)) < 0) {
        close(sockfd);
        perror("setsockopt (SO_BROADCAST)");
        throw;
    }

    // TODO setting interface for bat0
    #ifdef __arm__
        LOG_F(WARNING, "ARM architecture detected");
        memset(&b_iface, 0, sizeof(b_iface));
        snprintf(b_iface.ifr_name, sizeof(b_iface.ifr_name), b_iface_name);
        if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void)*&b_iface, sizeof(b_iface) < 0) {
            close(sockfd);
            perror("setsockopt (SO_BINDTODEVICE)");
            throw;
        }
    #else
     LOG_F(WARNING, "Standard architecture detected");
    #endif

    // binding to port
    if ( bind(sockfd, reinterpret_cast<const struct sockaddr*>(&_srvaddr), sizeof(_srvaddr)) < 0 ) {
        close(sockfd);
        perror("Cannot bind");
        throw;
    }
    LOG_F(INFO, "Broadcast socket listening on port %d", _port);
}


template<typename P>
void AbstractBroadcastNode<P>::_tr_receiver() {
    P packet;
    fd_set readfds, masterfds;

    loguru::set_thread_name( this->threadname(":receiver").c_str());
    LOG_F(INFO, "Starting %s receiver", _name);

    FD_ZERO(&masterfds);
    FD_ZERO(&readfds);
    FD_SET(sockfd, &masterfds);

    while (! process_stop) {
        sockaddr cli_addr;
        socklen_t len_cli_addr;
        timeval rcv_to = {1,0};

        memcpy(&readfds, &masterfds, sizeof(fd_set));

        if (select(sockfd+1, &readfds, nullptr, nullptr, &rcv_to) < 0) {
            perror("Cannot select");
            throw;
        }

        if ( FD_ISSET(sockfd, &readfds)) {
            if ( (recvfrom(sockfd, &packet, sizeof(P), 0, &cli_addr, &len_cli_addr) < 0) ) { 
                close(sockfd);
                perror("Cannot recvfrom");
                throw;
            }

            LOG_F(3, "Received packet: %s", packet.repr().c_str());
            if (_to_be_ignored(packet)){
                LOG_F(3, "Ignored packet: %s", packet.repr().c_str());
                continue;
            }
            _process_packet(packet);
        }
        /*else { //timeout }*/

        
    }
    LOG_F(INFO, "process_stop=true; exiting");
}


template<typename P>
inline std::string AbstractBroadcastNode<P>::threadname(std::string suffix) const {
    return std::string(_name)+":"+suffix;
}

template<typename P>
void AbstractBroadcastNode<P>::broadcast(const P& packet) const {
    socklen_t len_to_sockaddr = sizeof(sockaddr);
    if ( sendto(sockfd, &packet, sizeof(packet), 0, reinterpret_cast<const sockaddr*>(&_bc_sockaddr), len_to_sockaddr)  < 0 ) {
        close(sockfd);
        perror("Cannot sendto");
        throw;
    }
    LOG_F(3, "Sent packet: %s", packet.repr().c_str());
}

template<typename P>
inline bool AbstractBroadcastNode<P>::_to_be_ignored(const P& packet) const {
    return _nodeID == packet.nodeID;
}


template<typename P>
inline P AbstractBroadcastNode<P>::_produce_packet() {
    P packet;
    packet.nodeID = _nodeID;
    return packet;
}

template<typename P>
void AbstractBroadcastNode<P>::start() {
    LOG_F(WARNING, "Starting %s", _name);
    _setup_socket_bind();
    _thread_receiver = std::thread(&AbstractBroadcastNode::_tr_receiver, this);
}

template<typename P>
void AbstractBroadcastNode<P>::join() {
    _thread_receiver.join();
    LOG_F(WARNING, "BroadcastNode: joined all threads");
}


#endif
