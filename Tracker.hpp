#ifndef TRACKER_TRACKER_HPP
#define TRACKER_TRACKER_HPP
#include <thread>
#include <mutex>
#include <deque>
#include <map>
#include "packets.hpp"
#include "Position.hpp"
#include "loguru.hpp"
#include <unistd.h>

#define DEFAULT_TIMEOUT 30
#define TIMESTAMP_LAPS 30

class Tracker {
private :
    std::deque<Packet> _queue;
    std::map<unsigned short, std::pair<Position, uint32_t>> _hashmap;
    bool ALERT_PEER_LOST;
    unsigned short _timeout;
    std::thread _threadUpdateHash;
    std::thread _threadTimeCheck;

    void _changeFlag(bool);
    void _updateHashMap();
    void _checkTimestamp();

public :
    Tracker();
    Tracker(unsigned short timeout);
    void Notify(Packet paquet);
    ~Tracker();
};

#endif //TRACKER_TRACKER_HPP
