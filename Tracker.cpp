#include "Tracker.hpp"

Tracker::Tracker(unsigned short peer_lost_timeout, unsigned short period_mapcheck) : 
    _packetqueue(), _status_node_map(),
    _peer_lost_timeout(peer_lost_timeout), _period_mapcheck(period_mapcheck),
    _thread_check_node_map() {}


Tracker::~Tracker(){
    if(_thread_check_node_map.joinable()){
        _thread_check_node_map.join();
    }
}


void Tracker::_set_peer_lost_flag() {
    {
        std::lock_guard<std::mutex> lock(_mutex_peer_lost_flag);
        _ALERT_PEER_LOST = true;
    }
    LOG_F(3, "ALERT_PEER_LOST_FLAG set");
}


void Tracker::_reset_peer_lost_flag() {
    {
        std::lock_guard<std::mutex> lock(_mutex_peer_lost_flag);
        _ALERT_PEER_LOST = false;
    }
    LOG_F(3, "ALERT_PEER_LOST_FLAG reset");    
}


bool Tracker::is_peer_lost() {
    std::lock_guard<std::mutex> lock(_mutex_peer_lost_flag);
    return _ALERT_PEER_LOST;
}


void Tracker::notify(TrackPacket packet) {
    {
        std::lock_guard<std::mutex> lock(_mutex_packetqueue);
        _packetqueue.push(packet);
    }
    LOG_F(3, "Added packet to tracker  queue : %s", packet.repr().c_str());
}


void Tracker::_update_status_node_map(){
    TrackPacket packet;
    std::queue<TrackPacket> copy_queue;

    {
        std::lock_guard<std::mutex> lock(_mutex_packetqueue);
        if (not _packetqueue.empty()) {
            copy_queue = std::move(_packetqueue);
        }
    }

    while (! copy_queue.empty()){
        packet = copy_queue.front();
        copy_queue.pop();
        _status_node_map[packet.nodeID] = {packet.position, packet.timestamp};
        LOG_F(INFO, "Updated NodeID : %s", packet.repr().c_str());
    }
}


void Tracker::_tr_check_node_map(){
    bool call_set_peer_lost=false;
    uint32_t actualTimestamp;

    LOG_F(INFO, "Starting tracker _check_node_map");

    std::map<uint8_t, std::pair<Position, uint32_t>>::const_iterator it;

    while (! process_stop) {
        
        _update_status_node_map();

        {  // Scope spécifique sinon lock_guard va dans le sleep
            for (it = _status_node_map.cbegin(); it != _status_node_map.cend(); /*no increment*/ ) {
                actualTimestamp = static_cast<uint32_t>(std::time(nullptr));

                if ((actualTimestamp - std::get<1>(it->second)) > _peer_lost_timeout) {
                    //dead drone
                    LOG_F(WARNING, "Peer lost NodeID=%d", it->first);
                    _status_node_map.erase(it++);
                    call_set_peer_lost = true;
                } else {
                    ++it;
                }
            }

            if (call_set_peer_lost) {
                _set_peer_lost_flag();
                call_set_peer_lost = false;  // reset du booleen d'appel
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(_period_mapcheck));
    }
    LOG_F(INFO, "Tracker checkNodeMap - process_stop=true; exiting");
}


void Tracker::start() {
    loguru::set_thread_name("Tracker");
    _tr_check_node_map();
}

void Tracker::join() {
    _thread_check_node_map.join();
    LOG_F(WARNING, "Tracker: joined all threads");
}