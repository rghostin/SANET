#include "MainServer.hpp"

MainServer::MainServer(unsigned short port, uint8_t nodeID, Tracker &tracker, unsigned short heartTimer) : 
    _nodeID(nodeID), _heart_period(heartTimer),
    _port(port), sockfd(), _srvaddr(), _bc_sockaddr(),
    _mutex_next_seqnum(), _next_seqnum(0),
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


MainServer::~MainServer() {     // TODO research how to destroy gracefully
    if (_thread_receiver.joinable()) {
        _thread_receiver.join();
    }
    close(sockfd);
}


void MainServer::_setup_socket_bind() {
    // create socket
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Cannot create socket");
        throw;
    }

    int broadcast_flag = 1;  // TODO check rawad
    // Enable broadcast
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_flag, sizeof(broadcast_flag)) < 0) {
        close(sockfd);
        perror("setsockopt (SO_BROADCAST)");
        throw;
    }

    // setting interface for bat0
    // todo

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
    {   
        std::lock_guard<std::mutex> lock(_mutex_next_seqnum);
        uint32_t curr_timestamp = static_cast<uint32_t>(std::time(nullptr)); 
        packet = Packet(_nodeID, led_status, curr_timestamp, _next_seqnum++);
    }
    LOG_F(3, "Generated packet: " PACKET_FMT, PACKET_REPR(packet));
    return packet;    
}


inline bool MainServer::_to_be_ignored(const Packet& packet) const {
    return packet.nodeID == _nodeID;
}


void MainServer::_process_packet(const Packet& packet) const {
    LOG_F(INFO, "Processing packet: " PACKET_FMT, PACKET_REPR(packet));
    _tracker.notify(packet);
}


void MainServer::_hearbeat() {
    socklen_t len_to_sockaddr = sizeof(sockaddr);
    uint32_t curr_timestamp;

    loguru::set_thread_name("Server:Heartbeat");
    LOG_F(WARNING, "Starting heartbeat with period=%d", _heart_period);

    while (true) {
        curr_timestamp = static_cast<uint32_t>(std::time(nullptr));
        Packet packet = _produce_packet();

        if ( sendto(sockfd, &packet, sizeof(packet), 0, reinterpret_cast<const sockaddr*>(&_bc_sockaddr), len_to_sockaddr)  < 0 ) {
            close(sockfd);
            perror("Cannot sendto");
            throw;
        }

        LOG_F(INFO, "Sent hearbeat packet: " PACKET_FMT, PACKET_REPR(packet));
        sleep(_heart_period);
    }
}


void MainServer::_receiver() {
    Packet packet;

    loguru::set_thread_name("Server:Receiver");
    LOG_F(WARNING, "Starting server receiver");

    while (true) {
        sockaddr cli_addr;
        socklen_t len_cli_addr;

        if ( recvfrom(sockfd, &packet, sizeof(Packet), 0, &cli_addr, &len_cli_addr) < 0 ) {
            close(sockfd);
            perror("Cannot recvfrom");
            throw;
        }

        LOG_F(3, "Received packet: " PACKET_FMT, PACKET_REPR(packet));

        if (_to_be_ignored(packet)){
            continue;
        }

        _process_packet(packet);
    }
}


void MainServer::start() {
    _setup_socket_bind();
    _thread_receiver = std::thread(&MainServer::_receiver, this);
    _hearbeat();

    _thread_receiver.join();
}