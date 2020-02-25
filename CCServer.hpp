#ifndef __CCSERVER_HPP_
#define __CCSERVER_HPP_

#include <cstdio>
#include <thread>
#include <mutex>
#include <unistd.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <sys/types.h>  
#include <arpa/inet.h> 
#include <net/if.h>
#include <cstring> 
#include <string>
#include <sstream> 
#include <future>
#include <algorithm>
#include "loguru.hpp"
#include "common.hpp"
#include "settings.hpp"
#include "database_utils/DbUtils.hpp"
#include "CCCommands.hpp"
#include "utils.hpp"
#include "json_utils.hpp"

class CCServer final {
private:
    // Db
    const char* _path_db = DB_PATH;
    sqlite3 * _db;

    const unsigned short _port;
    const uint8_t _nodeID;

    int sockfd;
    sockaddr_in _srvaddr;
    ifreq b_iface;
    int _online_sockets[CC_MAX_CONNECTIONS] = {0};

    std::thread _thread_receiver;

    void _setup_socket();
    void _tr_receiver();
    void _dispatch(int, uint8_t);
    void _execute_fetch_all_pos(int);
    void _update_global_polygon(int);
    void _execute_fetch_map_number(int);
    void _execute_fetch_polygon(int);

public:
    CCServer(unsigned short port, uint8_t nodeID);
    CCServer(const CCServer&) = delete;
    CCServer(CCServer&&) = delete;
    CCServer& operator=(const CCServer&) = delete;
    CCServer& operator=(const CCServer&&) = delete;
    ~CCServer();

    void start();
    void join();
};

#endif