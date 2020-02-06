#include "TrackingServer.hpp"
#include "Tracker.hpp"

TrackingServer::TrackingServer(unsigned short port, uint8_t nodeID, Tracker &tracker, unsigned short heart_period) : 
    AbstractReliableBroadcastNode<TrackPacket>(nodeID, port, "TrackSrv", RELBC_PACKET_MAX_AGE),
    _heart_period(heart_period), _thread_heartbeat(),
    _tracker(tracker) 
    { 
    }


TrackingServer::~TrackingServer() {
    if (_thread_heartbeat.joinable()) {
        _thread_heartbeat.join();
    }
}


TrackPacket TrackingServer::_produce_packet() {
    TrackPacket packet = AbstractReliableBroadcastNode<TrackPacket>::_produce_packet();
    packet.led_status = false;
    packet.timestamp = static_cast<uint32_t>(std::time(nullptr)); 
    packet.position = _get_current_position();
    LOG_F(3, "Generated packet: %s", packet.repr().c_str());
    return packet;    
}


Position TrackingServer::_get_current_position() const {
    Position position(0,0);  // TODO construire pos actuelle
    return position;
}



void TrackingServer::_process_packet(const TrackPacket& packet) {
    AbstractReliableBroadcastNode<TrackPacket>::_process_packet(packet);
    _tracker.notify(packet);
}


void TrackingServer::_tr_hearbeat() {
    loguru::set_thread_name( this->threadname("heartbeat").c_str()); 
    LOG_F(INFO, "Starting %s heartbeat with period=%d", _name,_heart_period);

    while (! process_stop) {
        TrackPacket packet = _produce_packet();
        this->broadcast(packet);

        LOG_F(INFO, "Sent hearbeat packet: %s", packet.repr().c_str());
        std::this_thread::sleep_for(std::chrono::seconds(_heart_period));
    }
    LOG_F(INFO, "TrServer heartbeat process_stop=true; exiting");
}


void TrackingServer::start() {
    LOG_F(WARNING, "Starting Tracking server");
    AbstractReliableBroadcastNode<TrackPacket>::start();
    _tracker.start();
    _thread_heartbeat = std::thread(&TrackingServer::_tr_hearbeat, this);
}

void TrackingServer::join() {
    AbstractReliableBroadcastNode<TrackPacket>::join();
    _thread_heartbeat.join();
    _tracker.join();
    LOG_F(WARNING, "TrServer: joined all threads");
}