#include "ReliableMainServer.hpp"


bool ReliableMainServer::_is_already_processed(const Packet& packet) const {
    return true;
}


bool ReliableMainServer::_to_be_ignored(const Packet& packet) const {
    return MainServer::_to_be_ignored(packet) || _is_already_processed(packet);
}

void ReliableMainServer::_process_packet(const Packet& packet) const {
    socklen_t len_to_sockaddr = sizeof(sockaddr);

    MainServer::_process_packet(packet);

    LOG_F(INFO, "Echoing packet: " PACKET_FMT, PACKET_REPR(packet));
    if ( sendto(sockfd, &packet, sizeof(packet), 0, reinterpret_cast<const sockaddr*>(&_bc_sockaddr), len_to_sockaddr)  < 0 ) {
        close(sockfd);
        perror("Cannot sendto");
        throw;
    }
}