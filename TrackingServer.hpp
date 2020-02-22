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
#include <future>
#include "loguru.hpp"
#include "utils.hpp"
#include "settings.hpp"
#include "AbstractReliableBroadcast.hpp"
#include "common.hpp"
#include "TrackPacket.hpp"

typedef std::map<uint8_t, std::pair<Position, uint32_t>> nodemap_t;


class TrackingServer final : public AbstractReliableBroadcastNode<TrackPacket> {
private:
    // self information and settings
    const unsigned short _peer_lost_timeout = TRACKING_PEER_LOSS_TIMEOUT;

    // Unix Socket Sender
    const char* _usocket_path = FP_USOCKET_PATH;
    struct sockaddr_un _flight_server_addr;
    int _usockfd;
    void _setup_usocket();
    void _send_status_node_map(bool);
    std::string _get_json_nodemap(const nodemap_t& map, bool is_new_poly);


    std::mutex _mutex_status_node_map;
    nodemap_t _status_node_map;

    // threads
    std::thread _thread_check_node_map;
    void _tr_check_node_map();
    std::thread _thread_heartbeat;
    void _tr_heartbeat();

    TrackPacket _produce_packet() override;
    virtual void _process_packet(const TrackPacket&) override;  

    bool _received_first_poly=false;
    std::thread _thread_update_poly;
    void _tr_update_poly();

    std::mutex _mutex_json_global_poly;
    std::array<char, TRACKING_GLOBALPOLY_MAXBUF> _json_global_poly;
    uint16_t _polyid=0;

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
