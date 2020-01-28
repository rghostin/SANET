#include "Tracker.hpp"

std::mutex m;

Tracker::Tracker() : _queue(), _hashmap(), ALERT_PEER_LOST(false), _timeout(DEFAULT_TIMEOUT), _threadUpdateHash(&Tracker::_updateHashMap, this), _threadTimeCheck(&Tracker::_checkTimestamp, this) {}


Tracker::Tracker(unsigned short timeout) : _queue(), _hashmap(), ALERT_PEER_LOST(false), _timeout(timeout), _threadUpdateHash(&Tracker::_updateHashMap, this), _threadTimeCheck(&Tracker::_checkTimestamp, this) {}


//Add the received packet to the queue
void Tracker::Notify(Packet packet){
    _queue.push_back(packet);
    LOG_F(INFO, "[TRACKER] Added packet : {nodeID=%d, led_status=%d, timestamp=%d}", packet.nodeID, packet.led_status, packet.timestamp);
}

void Tracker::_updateHashMap(){
    Packet packet;
    uint32_t timestamp ;
    Position pos(0.f,0.f);  // POS by default

    while(true){
        if(_queue.size()>0){
            packet = _queue.front();
            _queue.pop_front();
            m.lock();
            _hashmap[packet.nodeID]=std::make_pair(pos, packet.timestamp);
            m.unlock();
            LOG_F(INFO, "[TRACKER] Updated NodeID : {nodeID=%d, led_status=%d, timestamp=%d}", packet.nodeID, packet.led_status, packet.timestamp);
        }
    }
}

void Tracker::_changeFlag(){
    m.lock();
    ALERT_PEER_LOST = true;
    m.unlock();
    LOG_F(INFO, "[TRACKER] ALERT_PEER_LOST - Flag fixed on True");
}

void Tracker::_resetFlag(){
    m.lock();
    ALERT_PEER_LOST = false;
    m.unlock();
    LOG_F(INFO, "[TRACKER] ALERT_PEER_LOST - Flag fixed on False");
}


void Tracker::_checkTimestamp(){
    m.lock();
    uint32_t actualTimestamp = std::time(nullptr);

    for (auto const&[key, val] : _hashmap ){
        if((actualTimestamp-std::get<1>(val))>_timeout){
            //dead drone
            _hashmap.erase(key);
            LOG_F(INFO, "[TRACKER] Removed NodeID : {nodeID=%d}", key);
            _changeFlag();
        }
    }
    m.unlock();
}

Tracker::~Tracker(){
    if(_threadUpdateHash.joinable()){
        _threadUpdateHash.join();
    }
    if(_threadTimeCheck.joinable()){
        _threadTimeCheck.join();
    }
}