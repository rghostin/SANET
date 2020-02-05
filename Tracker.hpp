#ifndef __TRACKER_HPP_
#define __TRACKER_HPP_

#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <unistd.h>
#include "loguru.hpp"
#include "common.hpp"
#include "packets.hpp"
#include "Position.hpp"


class Tracker final {
private :
    std::queue<TrackPacket> _packetqueue;
    std::map<uint8_t, std::pair<Position, uint32_t>> _status_node_map;
    bool _ALERT_PEER_LOST=false;
    unsigned short _peer_lost_timeout;
    unsigned short _period_mapcheck;

    // threads && mutex
    std::thread _thread_check_node_map;
    std::mutex _mutex_packetqueue;
    std::mutex _mutex_peer_lost_flag;

    void _set_peer_lost_flag();
    void _reset_peer_lost_flag();

    void _update_status_node_map();
    void _tr_check_node_map();

public :
    Tracker(unsigned short, unsigned short);
    Tracker(const Tracker&) = delete;
    Tracker(Tracker&&) = delete;
    Tracker& operator=(const Tracker&) = delete;
    Tracker& operator=(const Tracker&&) = delete;
    ~Tracker();

    void start();
    void notify(TrackPacket);
    bool is_peer_lost();
    void join();
};

#endif
