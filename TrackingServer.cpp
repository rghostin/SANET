#include "TrackingServer.hpp"

TrackingServer::TrackingServer(unsigned short port, uint8_t nodeID, Tracker &tracker, unsigned short heart_period) : 
    AbstractReliableBroadcastNode<Packet>(nodeID, port, RELBC_PACKET_MAX_AGE),
    _heart_period(heart_period),
    _tracker(tracker) 
    {
        _name = "TrackSrv";
    }


TrackingServer::~TrackingServer() {
    if (_thread_heartbeat.joinable()) {
        _thread_heartbeat.join();
    }
}



Packet TrackingServer::_produce_packet() {
    Packet packet = AbstractReliableBroadcastNode<Packet>::_produce_packet();
    packet.led_status = false;
    packet.timestamp = static_cast<uint32_t>(std::time(nullptr)); 
    packet.position = _get_current_position();
    LOG_F(3, "Generated packet: " PACKET_FMT, PACKET_REPR(packet));
    return packet;    
}


Position TrackingServer::_get_current_position() const {
    Position position(0,0);  // TODO construire pos actuelle
    return position;
}



void TrackingServer::_process_packet(const Packet& packet) {
    AbstractReliableBroadcastNode<Packet>::_process_packet(packet);
    _tracker.notify(packet);
}


void TrackingServer::_tr_hearbeat() {
    loguru::set_thread_name( (_name + ":heartbeat").c_str()); 
    LOG_F(WARNING, "Starting heartbeat with period=%d", _heart_period);

    while (! process_stop) {
        Packet packet = _produce_packet();
        this->broadcast(packet);

        LOG_F(INFO, "Sent hearbeat packet: " PACKET_FMT, PACKET_REPR(packet));
        std::this_thread::sleep_for(std::chrono::seconds(_heart_period));
    }
    LOG_F(INFO, "TrServer heartbeat process_stop=true; exiting");
}


void TrackingServer::start() {
    LOG_F(WARNING, "Starting Tracking server");
    AbstractReliableBroadcastNode<Packet>::start();
    _thread_heartbeat = std::thread(&TrackingServer::_tr_hearbeat, this);
}

void TrackingServer::join() {
    AbstractReliableBroadcastNode<Packet>::join();
    _thread_heartbeat.join();
    LOG_F(WARNING, "TrServer: joined all threads");
}