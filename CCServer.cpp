#include "CCServer.hpp"


 
CCServer::CCServer(unsigned short port) : _port(port)
{
    // setup reception sockaddr
    memset(&_srvaddr, 0, sizeof(_srvaddr));
    _srvaddr.sin_family = AF_INET;
    _srvaddr.sin_addr.s_addr = INADDR_ANY;
    _srvaddr.sin_port = htons(_port);
} 

CCServer::~CCServer() {
    if (_thread_welcome_receiver.joinable()) {
        _thread_welcome_receiver.join();
    }

    if (_thread_cli_receiver_gcollector.joinable()) {
        _thread_cli_receiver_gcollector.join();
    }

    if (close(sockfd) < 0) {
        perror("Cannot close socket");
        throw;
    }
}


void CCServer::_setup_socket() {
    // create socket
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Cannot create socket");
        throw;
    } 

    #ifdef __aarch64__
        LOG_F(WARNING, "ARM architecture detected");
        memset(&b_iface, 0, sizeof(b_iface));
        snprintf(b_iface.ifr_name, sizeof(b_iface.ifr_name), b_iface_name);
        if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void*)&b_iface, sizeof(b_iface)) < 0) {
            close(sockfd);
            perror("setsockopt (SO_BINDTODEVICE)");
            throw;
        }
    #else
     LOG_F(WARNING, "Standard architecture detected");
    #endif

    int opt=1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt (SO_REUSEADDR"); 
        throw; 
    } 

    // binding to port
    if ( bind(sockfd, reinterpret_cast<const struct sockaddr*>(&_srvaddr), sizeof(_srvaddr)) < 0 ) {
        close(sockfd);
        perror("Cannot bind");
        throw;
    }
    LOG_F(INFO, "Socket bound to port %d", _port);

    if (listen(sockfd, 3) < 0) { 
        perror("Cannot listen to port"); 
        throw; 
    } 
    LOG_F(INFO, "Socket listening to port %d", _port);
}



void CCServer::_tr_welcome_receiver() {
    fd_set readfds, masterfds;
    timeval rcv_timeout = {1,0}; 

    loguru::set_thread_name("CC:wlcRecv");

    FD_ZERO(&masterfds);
    FD_ZERO(&readfds);
    FD_SET(sockfd, &masterfds);

    while (! process_stop) {
        int connsockfd;
        sockaddr_in cliaddr;
        socklen_t len_cliaddr;

        memcpy(&readfds, &masterfds, sizeof(fd_set));

        if (select(sockfd+1, &readfds, nullptr, nullptr, &rcv_timeout) < 0) {
            perror("Cannot select");
            throw;
        }

        if ( ! FD_ISSET(sockfd, &readfds)) {
            // timeout
            continue;
        }

        if ( (connsockfd = accept(sockfd, reinterpret_cast<sockaddr*>(&cliaddr), &len_cliaddr)) < 0 ) {
            perror("Cannot accept connection");
            throw;
        }
        
        LOG_F(WARNING, "Connection from -- , starting client thread"); // TODO get IP of client
        std::thread cli_receiver(&CCServer::_tr_cli_receiver, this, connsockfd);
        LOG_F(INFO, "thread id is %lu", cli_receiver.get_id());
        {
            std::lock_guard<std::mutex> lock(_mutex_cli_receiver_threads_map);
            //auto it = _cli_receiver_threads_map.find(connsockfd);
            _cli_receiver_threads_map.insert(std::pair<int, std::thread>(connsockfd, std::move(cli_receiver)));   //[connsockfd] = std::move(cli_receiver);
            LOG_F(3, "thread inserted in map, current size: %d", _cli_receiver_threads_map.size());
        }
    }
    LOG_F(INFO, "process_stop=true; exiting");
}


void CCServer::_tr_cli_receiver(int connsockfd) {
    const char* msg = "hello";
    if (send(connsockfd, msg, strlen(msg)+1, 0) < 0) {
        perror("Cannot send");
        throw;
    }
    LOG_F(INFO, "Sent message hello");
}


void CCServer::_tr_cli_receiver_gcollector() {
    loguru::set_thread_name("CC:GC");

    while (! process_stop) {
        for (auto it=_cli_receiver_threads_map.begin(); it != _cli_receiver_threads_map.end(); /*no increment*/ ) {
            int connsockfd = it->first;
            std::thread& cli_thread = it->second;
            
            if (cli_thread.joinable()) {
                cli_thread.join();

                if (close(connsockfd) < 0) {
                    perror("Cannot close connsockfd");
                    throw;
                }

                {   // removing entry from threads map
                    std::lock_guard<std::mutex> lock(_mutex_cli_receiver_threads_map);
                    _cli_receiver_threads_map.erase(it++);
                    LOG_F(3, "thread removed from map, current size: %d", _cli_receiver_threads_map.size());
                }

            } else {
                ++it;
            }

        }
        sleep(_gc_period);
    }
    LOG_F(INFO, "process_stop=true; exiting");

}

void CCServer::start() {
    _setup_socket();
    _thread_cli_receiver_gcollector = std::thread(&CCServer::_tr_cli_receiver_gcollector, this);
    _thread_welcome_receiver = std::thread(&CCServer::_tr_welcome_receiver, this);
}

void CCServer::join() {
    _thread_welcome_receiver.join();
    _thread_cli_receiver_gcollector.join();
}