#include <cstdio>
#include <thread>
#include <mutex>
#include <unistd.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <net/if.h>
#include <cstring> 
#include <map>
#include "loguru.hpp"
#include "common.hpp"
#include "settings.hpp"


class CCServer final {
private:
    const unsigned short _port;
    const unsigned int _gc_period=CC_GC_PERIOD;

    int sockfd;
    sockaddr_in _srvaddr;
    ifreq b_iface;
    const char* b_iface_name=BATMAN_IFACE;

    std::mutex _mutex_cli_receiver_threads_map;
    std::map<int, std::thread> _cli_receiver_threads_map;

    std::thread _thread_welcome_receiver;
    std::thread _thread_cli_receiver_gcollector;

    void _setup_socket();

    void _tr_welcome_receiver();
    void _tr_cli_receiver(int connsockfd);
    void _tr_cli_receiver_gcollector();

public:
    CCServer(unsigned short port);
    CCServer(const CCServer&) = delete;
    CCServer(CCServer&&) = delete;
    CCServer& operator=(const CCServer&) = delete;
    CCServer& operator=(const CCServer&&) = delete;
    ~CCServer();

    void start();
    void join();
};