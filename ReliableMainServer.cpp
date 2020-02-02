#include "ReliableMainServer.hpp"


ReliableMainServer::ReliableMainServer(unsigned short port, uint8_t nodeID, Tracker &tracker, unsigned short heart_period, time_t max_packet_age) : 
    MainServer(port, nodeID, tracker, heart_period),
    _max_packet_age(max_packet_age)
    {}


ReliableMainServer::~ReliableMainServer() {
    if (_thread_update_last_seq_map.joinable()) {
        _thread_update_last_seq_map.join();
    }
    // ~MainServer called automatically
}


bool ReliableMainServer::_is_already_processed(const Packet& packet) const {
    std::lock_guard<std::mutex> lock(_mutex_last_seq_map);
    const std::map<uint8_t, std::pair<uint32_t, unsigned int>>::const_iterator it = _last_seq_map.find(packet.nodeID);
    if (it != _last_seq_map.end()) {
        const uint32_t& seqnum = (it->second).first;
        const uint32_t& age = (it->second).second;
        return (packet.seqnum <= seqnum || age <= 0);
    }
    LOG_F(3, "Packet NOT already processed" PACKET_FMT, PACKET_REPR(packet));
    return false;
}


bool ReliableMainServer::_to_be_ignored(const Packet& packet) const {
    return MainServer::_to_be_ignored(packet) || _is_already_processed(packet);
}


void ReliableMainServer::_process_packet(const Packet& packet)  {
    /* only called if _to_be_ignored is false */
    socklen_t len_to_sockaddr = sizeof(sockaddr);

    MainServer::_process_packet(packet);

    {
        std::lock_guard<std::mutex> lock(_mutex_last_seq_map);
         _last_seq_map[packet.nodeID] = std::pair<uint32_t, unsigned int>(packet.seqnum, _max_packet_age);
    }

    // in addition to the standard server, echo a broadcast of the packet for reliable broadcast
    LOG_F(INFO, "Echoing packet: " PACKET_FMT, PACKET_REPR(packet));
    if ( sendto(sockfd, &packet, sizeof(packet), 0, reinterpret_cast<const sockaddr*>(&_bc_sockaddr), len_to_sockaddr)  < 0 ) {
        close(sockfd);
        perror("Cannot sendto");
        throw;
    }


    
}


void ReliableMainServer::_tr_update_last_seq_map() {
    loguru::set_thread_name("RelMainServer:PacketLastSeq");
    while (! process_stop) {
        {
            std::lock_guard<std::mutex> lock(_mutex_last_seq_map);
            if (not _last_seq_map.empty()) {
                LOG_F(3, "_last_seq_map size: %lu", _last_seq_map.size());
                for (auto it=_last_seq_map.begin(); it != _last_seq_map.end(); ++it) {
                    unsigned int& age = (it->second).second;
                    if (age <= 0) {
                        LOG_F(3, "Erasing from _last_seq_map nodeID=%d", it->first);
                        it = _last_seq_map.erase(it);
                    } else {
                        age--;
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LOG_F(INFO, "RMS updateLastSeq process_stop=true; exiting");
}

void ReliableMainServer::start() {
    MainServer::start();
    _thread_update_last_seq_map = std::thread(&ReliableMainServer::_tr_update_last_seq_map, this);
}

void ReliableMainServer::join() {
    _thread_update_last_seq_map.join();
    MainServer::join();
    LOG_F(WARNING, "ReliableMainServer: joined all threads");

}
