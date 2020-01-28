#include "Dispatcher.hpp"

Dispatcher::Dispatcher(unsigned short port, unsigned short nodeID, Tracker *tracker) : _port(port), sockfd(), srvaddr(), _nodeId(nodeID), _tracker(tracker), _threadHeartBeat(&Dispatcher::_hearbeat, this), _heartTimer(DEFAULTHEARTTIMER) {
    if (not _tracker) {
        LOG_F(WARNING, "Tracker provided is NULLPTR");
    }
}


Dispatcher::Dispatcher(unsigned short port, unsigned short nodeID, Tracker *tracker, unsigned short heartTimer) : _port(port), sockfd(), srvaddr(), _nodeId(nodeID), _tracker(tracker), _threadHeartBeat(&Dispatcher::_hearbeat, this), _heartTimer(heartTimer) {
    if (not _tracker) {
        LOG_F(WARNING, "Tracker provided is NULLPTR");
    }
}


Dispatcher::~Dispatcher() {
    if (_threadHeartBeat.joinable()) {
        _threadHeartBeat.join();
    }
    close(sockfd);
    delete _tracker;  // A voir si on laisse le delete ici ou directement dans ROBIN.CPP TODO
    _tracker = nullptr;
}


void Dispatcher::_setup_socket() {
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

    // filling socket information for reception
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = INADDR_ANY;
    srvaddr.sin_port = htons(_port);

    // binding to port
    if ( bind(sockfd, reinterpret_cast<const struct sockaddr*>(&srvaddr), sizeof(srvaddr)) < 0 ) {
        close(sockfd);
        perror("Cannot bind");
        throw;
    }
    LOG_F(WARNING, "Dispatcher socket listening on port %d", _port);
}


void Dispatcher::start() {
    Packet packet;

    loguru::set_thread_name("Dispatcher");
    _setup_socket();

    while (true) {
        sockaddr cli_addr;
        socklen_t len_cli_addr;

        if ( recvfrom(sockfd, &packet, sizeof(Packet), 0, &cli_addr, &len_cli_addr) < 0 ) {
            close(sockfd);
            perror("Cannot recvfrom");
            throw;
        }

        LOG_F(INFO, "Received packet : {nodeID=%d, led_status=%d}", packet.nodeID, packet.led_status);
    }
}

void Dispatcher::_hearbeat() {
    Packet packet;
    packet.nodeID = _nodeId;

    sockaddr_in to_sockaddr;
    socklen_t len_to_sockaddr = sizeof(sockaddr);

    // fill receiver address
    memset(&to_sockaddr, 0, sizeof(to_sockaddr));
    to_sockaddr.sin_family = AF_INET;
    to_sockaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); //inet_addr("192.168.1.33"); //INET_BROADCAST  // todo fix
    to_sockaddr.sin_port = htons(_port);

    while (true) {
        packet.timestamp = std::time(nullptr);

        if ( sendto(sockfd, &packet, sizeof(packet), 0, reinterpret_cast<const sockaddr*>(&to_sockaddr), len_to_sockaddr)  < 0 ) {
            close(sockfd);
            perror("Cannot sendto");
            throw;
        }

        LOG_F(INFO, "Sent packet : {nodeID=%d, led_status=%d, timestamp=%d}", packet.nodeID, packet.led_status, packet.timestamp);
        sleep(_heartTimer);
    }
}