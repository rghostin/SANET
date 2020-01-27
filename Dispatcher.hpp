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
#include "packets.hpp"
#include "loguru.hpp"


#define MAXBUFSIZE 1024     // todo recheck according to img size


class Dispatcher final {
private:
    const unsigned short _port;
    int sockfd;
    sockaddr_in srvaddr;

    void _setup_socket();
    
public:
    Dispatcher(unsigned short port);
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher(Dispatcher&&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&&) = delete;
    ~Dispatcher();

    void start();
};

#endif
