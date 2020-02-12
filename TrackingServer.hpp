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
#include <sys/un.h>
#include "loguru.hpp"
#include "settings.hpp"
#include "AbstractReliableBroadcast.hpp"
#include "common.hpp"
#include "packets.hpp"

typedef std::map<uint8_t, std::pair<Position, uint32_t>> nodemap_t;


class TrackingServer final : public AbstractReliableBroadcastNode<TrackPacket> {
private:
    // self information and settings
    const unsigned short _heart_period=TRACKING_HEARTBEAT_PERIOD;
    const unsigned short _peer_lost_timeout = TRACKING_PEER_LOSS_TIMEOUT;
    const unsigned short _period_mapcheck = TRACKING_PERIOD_CHECK_NODEMAP;

    // Unix Socket Sender
    const char* _usocket_path = FP_USOCKET_PATH;
    struct sockaddr_un _flight_server_addr;
    int _usockfd;
    void _setup_usocket();
    void _send_status_node_map();

    std::mutex _mutex_status_node_map;
    nodemap_t _status_node_map;

    // threads
    std::thread _thread_check_node_map;
    void _tr_check_node_map();
    std::thread _thread_heartbeat;
    void _tr_hearbeat();

    TrackPacket _produce_packet() override;
    virtual void _process_packet(const TrackPacket&) override;  
    Position _get_current_position() const;

public:
    TrackingServer(unsigned short port, uint8_t nodeID);
    TrackingServer(const TrackingServer&) = delete;
    TrackingServer(TrackingServer&&) = delete;
    TrackingServer& operator=(const TrackingServer&) = delete;
    TrackingServer& operator=(const TrackingServer&&) = delete;
    virtual ~TrackingServer();

    virtual void start() override;
    virtual void join() override;
};

#endif
