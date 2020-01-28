#include "Tracker.hpp"

std::mutex m;

Tracker::Tracker() : _queue(), _hashmap(), ALERT_PEER_LOST(false), _timeout(DEFAULT_TIMEOUT), _threadUpdateHash(&Tracker::_updateHashMap, this), _threadTimeCheck(&Tracker::_checkTimestamp, this) {
    std::thread addToQueue(&Tracker::_updateHashMap, this);
    std::thread timestampChecker(&Tracker::_checkTimestamp, this);
}

Tracker::Tracker(unsigned short timeout) : _queue(), _hashmap(), ALERT_PEER_LOST(false), _timeout(timeout), _threadUpdateHash(&Tracker::_updateHashMap, this), _threadTimeCheck(&Tracker::_checkTimestamp, this) {
}


//Add the received packet to the queue
void Tracker::Notify(Packet packet){
    _queue.push_back(packet);
}

void Tracker::_updateHashMap(){
    Packet paquet;
    uint32_t timestamp ;
    Position pos(0.f,0.f);
    while(true){
        if(_queue.size()>0){
            paquet = _queue.front();
            _queue.pop_front();
            m.lock();
            _hashmap[paquet.nodeID]=std::make_pair(pos, paquet.timestamp);
            m.unlock();
        }
    }
}

void Tracker::_changeFlag(){
    m.lock();
    ALERT_PEER_LOST = true;
    m.unlock();
}

void Tracker::_resetFlag(){
    m.lock();
    ALERT_PEER_LOST = false;
    m.unlock();
}


void Tracker::_checkTimestamp(){
    m.lock();
    uint32_t actualTimestamp = std::time(nullptr);
    for (auto const&[key, val] : _hashmap ){
        if((actualTimestamp-std::get<1>(val))>_timeout){
            //dead drone
            _hashmap.erase(key);
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