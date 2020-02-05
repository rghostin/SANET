#ifndef __TRACKINGSERVER_HPP_
#define __TRACKINGSERVER_HPP_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <thread>
#include <mutex>
#include "loguru.hpp"
#include "settings.hpp"
#include "AbstractReliableBroadcast.hpp"
#include "common.hpp"
#include "packets.hpp"
#include "Tracker.hpp"


class TrackingServer final : public AbstractReliableBroadcastNode<TrackPacket> {
private:
    // self information and settings
    unsigned short _heart_period;

    // delegations
    Tracker& _tracker;
    std::thread _thread_tracker;
    std::thread _thread_heartbeat;
    void _tr_hearbeat();

    TrackPacket _produce_packet() override;
    virtual void _process_packet(const TrackPacket&) override;  
    Position _get_current_position() const;

public:
    TrackingServer(unsigned short port, uint8_t nodeID, Tracker &tracker, unsigned short heart_period=TRACKING_HEARTBEAT_PERIOD);
    TrackingServer(const TrackingServer&) = delete;
    TrackingServer(TrackingServer&&) = delete;
    TrackingServer& operator=(const TrackingServer&) = delete;
    TrackingServer& operator=(const TrackingServer&&) = delete;
    virtual ~TrackingServer();

    virtual void start() override;
    virtual void join() override;
};

#endif
