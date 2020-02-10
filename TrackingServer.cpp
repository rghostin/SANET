#include "TrackingServer.hpp"

TrackingServer::TrackingServer(unsigned short port, uint8_t nodeID) : 
    AbstractReliableBroadcastNode<TrackPacket>(nodeID, port, "TrackSrv"),
     _mutex_status_node_map(),_status_node_map(), _thread_check_node_map(),_thread_heartbeat() {}

 
TrackingServer::~TrackingServer() {
    if (_thread_heartbeat.joinable()) {
        _thread_heartbeat.join();
    }

    if (_thread_check_node_map.joinable()) {
        _thread_check_node_map.join();
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
    { // TODO - need for queue to make asynchrone ?
        std::lock_guard<std::mutex> lock(_mutex_status_node_map);
        _status_node_map[packet.nodeID] = {packet.position, packet.timestamp};
    }
    LOG_F(INFO, "Updated NodeID : %s", packet.repr().c_str());
    LOG_F(7, "status_node_map:\n%s", print_log_map(_status_node_map).c_str());
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



void TrackingServer::_tr_check_node_map(){
    bool need_fp_recompute=false;
    uint32_t curr_timestamp;

    loguru::set_thread_name(this->threadname(":chkNodeMap").c_str()); // TODO
    LOG_F(INFO, "Starting _check_node_map");

    std::map<uint8_t, std::pair<Position, uint32_t>>::const_iterator it;

    while (! process_stop) {
        { 
            for (it = _status_node_map.cbegin(); it != _status_node_map.cend(); /*no increment*/ ) {
                curr_timestamp = static_cast<uint32_t>(std::time(nullptr));

                if ((curr_timestamp - std::get<1>(it->second)) > _peer_lost_timeout) {
                    //dead drone
                    LOG_F(WARNING, "Peer lost NodeID=%d", it->first);
                    _status_node_map.erase(it++);
                    need_fp_recompute = true;
                } else {
                    ++it;
                }
            }

            if (need_fp_recompute) {
                need_fp_recompute = false;  // reset for next iter
                // TODO delegation to flight planner
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(_period_mapcheck));
    }
    LOG_F(INFO, "Tracker checkNodeMap - process_stop=true; exiting");
}



void TrackingServer::start() {
    LOG_F(WARNING, "Starting Tracking server");
    AbstractReliableBroadcastNode<TrackPacket>::start();
    _thread_check_node_map = std::thread(&TrackingServer::_tr_check_node_map, this);
    _thread_heartbeat = std::thread(&TrackingServer::_tr_hearbeat, this);
}

void TrackingServer::join() {
    AbstractReliableBroadcastNode<TrackPacket>::join();
    _thread_heartbeat.join();
    _thread_check_node_map.join();
    LOG_F(WARNING, "TrServer: joined all threads");
}