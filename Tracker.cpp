#include "Tracker.hpp"

std::mutex mutex_status_node_map;
std::mutex mutex_peer_lost_flag;


Tracker::Tracker() : _packetqueue(), _status_node_map(), ALERT_PEER_LOST(false), _peer_lost_timeout(DEFAULT_TIMEOUT), _thread_update_statusNodeMap(), _thread_checktimestamp() {}


Tracker::Tracker(unsigned short timeout) : _packetqueue(), _status_node_map(), ALERT_PEER_LOST(false), _peer_lost_timeout(timeout), _thread_update_statusNodeMap(), _thread_checktimestamp() {}


void Tracker::start() {
    _thread_update_statusNodeMap = std::thread(&Tracker::_update_status_node_map, this);
    _thread_checktimestamp = std::thread(&Tracker::_checkTimestamp, this);

    loguru::set_thread_name("Tracker");
}


//Add the received packet to the queue
void Tracker::notify(Packet packet){
    _packetqueue.push(packet);
    LOG_F(INFO, "Added packet : {nodeID=%d, led_status=%d, timestamp=%d}", packet.nodeID, packet.led_status, packet.timestamp);
}


void Tracker::_update_status_node_map(){
    Packet packet;
    Position pos = {0, 0};  // POS by default

    while (true) {
        if (_packetqueue.size() > 0) {
            packet = _packetqueue.front();
            _packetqueue.pop();
            mutex_status_node_map.lock();
            _status_node_map[packet.nodeID] = {pos, packet.timestamp};
            mutex_status_node_map.unlock();
            LOG_F(INFO, "Updated NodeID : {nodeID=%d, led_status=%d, timestamp=%d}", packet.nodeID, packet.led_status, packet.timestamp);
        }
    }
}


void Tracker::_set_peer_lostFlag() {
    mutex_peer_lost_flag.lock();
    ALERT_PEER_LOST = true;
    mutex_peer_lost_flag.unlock();

    // TODO RECALCULER l'area
    _reset_peer_lostFlag();  // On reset après avoir pris en compte les modifications
}


void Tracker::_reset_peer_lostFlag() {
    mutex_peer_lost_flag.lock();
    ALERT_PEER_LOST = false;
    mutex_peer_lost_flag.unlock();
}


void Tracker::_checkTimestamp(){
    uint32_t actualTimestamp = std::time(nullptr);

    while (true) {
        mutex_status_node_map.lock();
        for (auto const&[key, val] : _status_node_map ) {
            if ((actualTimestamp - std::get<1>(val)) >_peer_lost_timeout) {
                //dead drone
                _status_node_map.erase(key);
                LOG_F(WARNING, "Peer lost with the following NodeID : {nodeID=%d}", key);
                _set_peer_lostFlag();
            }
        }
        mutex_status_node_map.unlock();
        sleep(TIMESTAMP_LAPS);
    }
}


Tracker::~Tracker(){
    if(_thread_update_statusNodeMap.joinable()){
        _thread_update_statusNodeMap.join();
    }

    if(_thread_checktimestamp.joinable()){
        _thread_checktimestamp.join();
    }
}