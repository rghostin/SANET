#include "Tracker.hpp"

std::mutex mutex_status_node_map;
std::mutex mutex_packetqueue;
std::mutex mutex_peer_lost_flag;


Tracker::Tracker() : _packetqueue(), _status_node_map(), ALERT_PEER_LOST(false), _peer_lost_timeout(DEFAULT_TIMEOUT), _thread_update_statusNodeMap(), _thread_checktimestamp() {}


Tracker::Tracker(unsigned short timeout) : _packetqueue(), _status_node_map(), ALERT_PEER_LOST(false), _peer_lost_timeout(timeout), _thread_update_statusNodeMap(), _thread_checktimestamp() {}


void Tracker::start() {
    _thread_update_statusNodeMap = std::thread(&Tracker::_update_status_node_map, this);
    _thread_checktimestamp = std::thread(&Tracker::_checkTimestamp, this);
}


//Add the received packet to the queue
void Tracker::notify(Packet packet) {
    std::lock_guard<std::mutex> lock(mutex_packetqueue);
    _packetqueue.push(packet);
    LOG_F(INFO, "Added packet : %s", get_packetInfos(packet).c_str());
}


void Tracker::_update_status_node_map(){
    Packet packet;
    Position pos = {0, 0};  // POS by default
    loguru::set_thread_name("Tracker");
    LOG_F(INFO, "Startup of _update_status_node_map");

    while (true) {
        if (_packetqueue.size() > 0) {
            std::lock_guard<std::mutex> lock(mutex_packetqueue);
            packet = _packetqueue.front();
            _packetqueue.pop();
            std::lock_guard<std::mutex> lock2(mutex_status_node_map);
            _status_node_map[packet.nodeID] = {pos, packet.timestamp};
            LOG_F(INFO, "Updated NodeID : %s", get_packetInfos(packet).c_str());
        }

    }
}


void Tracker::_set_peer_lostFlag() {
    std::lock_guard<std::mutex> lock(mutex_peer_lost_flag);
    ALERT_PEER_LOST = true;
}


void Tracker::_reset_peer_lostFlag() {
    std::lock_guard<std::mutex> lock(mutex_peer_lost_flag);
    ALERT_PEER_LOST = false;
}


bool Tracker::is_peer_lost() {
    std::lock_guard<std::mutex> lock(mutex_peer_lost_flag);
    return ALERT_PEER_LOST;
}


void Tracker::_checkTimestamp(){
    uint32_t actualTimestamp = std::time(nullptr);
    loguru::set_thread_name("Tracker");
    LOG_F(INFO, "Startup of _checkTimestamp");

    while (true) {
        {  // Scope spécifique sinon lock_guard va dans le sleep
            std::lock_guard<std::mutex> lock(mutex_status_node_map);
            for (auto const&[key, val] : _status_node_map ) {
                if ((actualTimestamp - std::get<1>(val)) >_peer_lost_timeout) {
                    //dead drone
                    _status_node_map.erase(key);
                    LOG_F(WARNING, "Peer lost with the following NodeID : {nodeID=%d}", key);
                    _set_peer_lostFlag();
                }
            }
        }
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