#include "MainServer.hpp"

MainServer::MainServer(unsigned short port, uint8_t nodeID, Tracker &tracker, unsigned short heart_period) : 
    _port(port),
    _nodeID(nodeID), _heart_period(heart_period),
    _tracker(tracker) 
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


MainServer::~MainServer() {
    if (_thread_receiver.joinable()) {
        _thread_receiver.join();
    }

    if (_thread_heartbeat.joinable()) {
        _thread_heartbeat.join();
    }
    close(sockfd);
}


void MainServer::_setup_socket_bind() {
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
    // 

    // binding to port
    if ( bind(sockfd, reinterpret_cast<const struct sockaddr*>(&_srvaddr), sizeof(_srvaddr)) < 0 ) {
        close(sockfd);
        perror("Cannot bind");
        throw;
    }
    LOG_F(WARNING, "Server socket listening on port %d", _port);
}


Packet MainServer::_produce_packet(bool led_status) {
    Packet packet;
    Position position = _get_current_position();
    {   
        std::lock_guard<std::mutex> lock(_mutex_next_seqnum);
        uint32_t curr_timestamp = static_cast<uint32_t>(std::time(nullptr)); 
        packet = Packet(_nodeID, led_status, curr_timestamp, _next_seqnum++, position);
    }
    LOG_F(3, "Generated packet: " PACKET_FMT, PACKET_REPR(packet));
    return packet;    
}


Position MainServer::_get_current_position() const {
    Position position;  // TODO construire pos actuelle
    return position;
}


inline bool MainServer::_to_be_ignored(const Packet& packet) const {
    return packet.nodeID == _nodeID;
}


void MainServer::_process_packet(const Packet& packet) {
    LOG_F(INFO, "Processing packet: " PACKET_FMT, PACKET_REPR(packet));
    _tracker.notify(packet);
}


void MainServer::_tr_hearbeat() {
    socklen_t len_to_sockaddr = sizeof(sockaddr);

    loguru::set_thread_name("MainServer:Heartbeat");
    LOG_F(WARNING, "Starting heartbeat with period=%d", _heart_period);

    while (! process_stop) {
        Packet packet = _produce_packet();

        if ( sendto(sockfd, &packet, sizeof(packet), 0, reinterpret_cast<const sockaddr*>(&_bc_sockaddr), len_to_sockaddr)  < 0 ) {
            close(sockfd);
            perror("Cannot sendto");
            throw;
        }

        LOG_F(INFO, "Sent hearbeat packet: " PACKET_FMT, PACKET_REPR(packet));
        std::this_thread::sleep_for(std::chrono::seconds(_heart_period));
    }
    LOG_F(INFO, "MS heartbeat process_stop=true; exiting");
}


void MainServer::_tr_receiver() {
    Packet packet;
    int sts_recv;
    fd_set readfds, masterfds;

    loguru::set_thread_name("MainServer:Receiver");
    LOG_F(WARNING, "Starting server receiver");

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

        if (! FD_ISSET(sockfd, &readfds)) {
            // timeout
            continue;
        }

        sts_recv = recvfrom(sockfd, &packet, sizeof(Packet), 0, &cli_addr, &len_cli_addr);
        if (sts_recv == EAGAIN || sts_recv==EWOULDBLOCK) {
            printf("to\n");
        }

        if ( sts_recv <= 0) { // if error excluding timeout
            printf("%d\n", sts_recv);
            close(sockfd);
            perror("Cannot recvfrom");
            throw;
        }

        LOG_F(3, "Received packet: " PACKET_FMT, PACKET_REPR(packet));
        if (_to_be_ignored(packet)){
            LOG_F(3, "Ignored packet: " PACKET_FMT, PACKET_REPR(packet));
            continue;
        }
        _process_packet(packet);
    }
    LOG_F(INFO, "MS receiver - process_stop=true; exiting");
}


void MainServer::start() {
    LOG_F(WARNING, "Starting server");
    _setup_socket_bind();
    _thread_receiver = std::thread(&MainServer::_tr_receiver, this);
    _thread_heartbeat = std::thread(&MainServer::_tr_hearbeat, this);
}

void MainServer::join() {
    _thread_heartbeat.join();
    _thread_receiver.join();
    LOG_F(WARNING, "MainServer: joined all threads");
}