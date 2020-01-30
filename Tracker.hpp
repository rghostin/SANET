#ifndef __TRACKER_HPP_
#define __TRACKER_HPP_

#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include "packets.hpp"
#include "Position.hpp"
#include "loguru.hpp"
#include <unistd.h>

class Tracker {
private :
    std::queue<Packet> _packetqueue;
    std::map<unsigned short, std::pair<Position, uint32_t>> _status_node_map;
    bool ALERT_PEER_LOST;
    unsigned short _peer_lost_timeout;
    unsigned short _intervalTime_checkTimestamp;
    std::thread _thread_update_statusNodeMap;
    std::thread _thread_checktimestamp;

    void _set_peer_lostFlag();
    void _reset_peer_lostFlag();
    void _update_status_node_map();
    void _checkTimestamp();

public :
    Tracker() = delete;
    Tracker(unsigned short, unsigned short);
    void start();
    void notify(Packet);
    bool is_peer_lost();
    ~Tracker();
};

#endif
