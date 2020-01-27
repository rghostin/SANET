#include "Dispatcher.hpp"


Dispatcher::Dispatcher(unsigned short port) : _port(port), sockfd(), srvaddr() {}


Dispatcher::~Dispatcher() {
    close(sockfd);
}


void Dispatcher::_setup_socket() {
    // create socket
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Cannot create socket");
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
