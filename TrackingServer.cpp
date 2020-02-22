#include "TrackingServer.hpp"

// JSON UTILS ===================================

std::string TrackingServer::_get_json_nodemap(const nodemap_t& map) {
    char buffer[4096]="";
    std::string res = "{";
    
    Position _mypos =  _get_current_position();

    for (auto it=map.cbegin(); it!=map.cend(); ++it) {
        uint32_t nodeid = it->first;
        Position pos = std::get<0>(it->second);

        snprintf(buffer, sizeof(buffer), "\"%u\": [%f, %f]", nodeid, pos.longitude, pos.latitude);
        res += buffer;
        res += ",";
        memset(buffer, 0, sizeof(buffer));
    }
    snprintf(buffer, sizeof(buffer), "\"%u\": [%f, %f]", _nodeID, _mypos.longitude, _mypos.latitude);
    res += buffer;
    res += "}";
    return res;
}
//=================================================



TrackingServer::TrackingServer(unsigned short port, uint8_t nodeID) : 
    AbstractReliableBroadcastNode<TrackPacket>(nodeID, port, "TrackSrv"),
     _flight_server_addr(), _usockfd(), _mutex_status_node_map(),_status_node_map(),
     _thread_check_node_map(),_thread_heartbeat(), _mutex_json_global_poly(), _json_global_poly() {}

 
TrackingServer::~TrackingServer() {
    if (_thread_heartbeat.joinable()) {
        _thread_heartbeat.join();
    }

    if (_thread_check_node_map.joinable()) {
        _thread_check_node_map.join();
    }

    if (_thread_update_poly.joinable()) {
        _thread_update_poly.join();
    }
} 


TrackPacket TrackingServer::_produce_packet() {
    TrackPacket packet = AbstractReliableBroadcastNode<TrackPacket>::_produce_packet();
    packet.led_status = false;
    packet.position = _get_current_position();
    {
        std::lock_guard<std::mutex> lock(_mutex_json_global_poly);
        packet.globalpoly = _json_global_poly;   // copy
        packet.polyid = _polyid;
    }
    LOG_F(3, "Generated packet: %s", packet.repr().c_str());
    return packet;    
}



void TrackingServer::_process_packet(const TrackPacket& packet) {
    AbstractReliableBroadcastNode<TrackPacket>::_process_packet(packet);    
    bool new_node=false;
    { // TODO - need for queue to make asynchrone ?
        std::lock_guard<std::mutex> lock(_mutex_status_node_map);
        new_node = (_status_node_map.find(packet.nodeID) == _status_node_map.end());
        _status_node_map[packet.nodeID] = {packet.position, packet.timestamp};
        dbInsertOrUpdateNode(_db, packet.nodeID, packet.position, packet.timestamp);
    }
    if (new_node) {
        LOG_F(WARNING, "New NodeID=%d", packet.nodeID);
        // on new node
        {
            std::lock_guard<std::mutex> lock(mutex_new_poly);
            if (_received_first_poly) {
                _send_status_node_map();

            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(_mutex_json_global_poly);
        if (packet.polyid > _polyid) {
            _polyid = packet.polyid;
            json_write_poly_to_file(packet.globalpoly.data(), FP_GLOBAL_AREA_POLYGON_PATH);
        }   
    }
    
    LOG_F(INFO, "Updated NodeID : %s", packet.repr().c_str());
    LOG_F(7, "status_node_map:\n%s", print_log_map(_status_node_map).c_str());
}


void TrackingServer::_tr_heartbeat() {
    Position lastpos;

    loguru::set_thread_name( this->threadname("heartbeat").c_str()); 
    LOG_F(INFO, "Starting heartbeat");

    while (! process_stop) {
        Position currpos = _get_current_position();
        if (true) { // (currpos != lastpos) { 
            lastpos = currpos;
            TrackPacket packet = _produce_packet();
            this->broadcast(packet);
            LOG_F(INFO, "Sent hearbeat packet: %s", packet.repr().c_str());
        }

        std::this_thread::sleep_for(std::chrono::seconds(static_cast<int>(FP_AUTOPILOT_SPEED)));        
    }
    LOG_F(INFO, "TrServer heartbeat process_stop=true; exiting");
}

// Node Map Sender ===========================================

void TrackingServer::_setup_usocket(){
    if ( (_usockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        throw;
    }

    memset(&_flight_server_addr, 0, sizeof(_flight_server_addr));
    _flight_server_addr.sun_family = AF_UNIX;
    if (*_usocket_path == '\0') {
        *_flight_server_addr.sun_path = '\0';
        strncpy(_flight_server_addr.sun_path+1, _usocket_path+1, sizeof(_flight_server_addr.sun_path)-2);
    } else {
        strncpy(_flight_server_addr.sun_path, _usocket_path, sizeof(_flight_server_addr.sun_path)-1);
    }
    if (connect(_usockfd, reinterpret_cast<sockaddr*>(&_flight_server_addr), sizeof(_flight_server_addr)) == -1) {
        perror("connect error with flight server");
        throw;
    }
    LOG_F(INFO, "Usocket setup");
}

void TrackingServer::_send_status_node_map(){
    _setup_usocket();
    std::string json_nodemap;
    {
        std::lock_guard<std::mutex> lock(_mutex_status_node_map);
        json_nodemap = _get_json_nodemap(_status_node_map);
    }
    if (send(_usockfd, json_nodemap.c_str(), json_nodemap.length(), 0) < 0) {
        perror("Cannot send the node map");
    }
    LOG_F(WARNING, "Node map sent: %s", json_nodemap.c_str());
    close(_usockfd);

}
// ===========================================================


void TrackingServer::_tr_check_node_map(){
    bool need_fp_recompute=false;
    uint32_t curr_timestamp;

    loguru::set_thread_name(this->threadname(":chkNodeMap").c_str());
    LOG_F(INFO, "Starting _check_node_map");

    nodemap_t::const_iterator it;

    while (! process_stop) {
        { 
            std::lock_guard<std::mutex> lock(_mutex_status_node_map);

            for (it = _status_node_map.cbegin(); it != _status_node_map.cend(); /*no increment*/ ) {
                curr_timestamp = static_cast<uint32_t>(std::time(nullptr));

                if ((curr_timestamp - std::get<1>(it->second)) > _peer_lost_timeout) {
                    //dead drone
                    uint8_t nodeid = it->first;
                    LOG_F(WARNING, "Peer lost NodeID=%d", nodeid);
                    _status_node_map.erase(it++);
                    dbRemoveNode(_db, nodeid);
                    need_fp_recompute = true;
                } else {
                    ++it;
                }
            }
        }

        if (need_fp_recompute) {
            need_fp_recompute = false;  // reset for next iter

            {
            std::lock_guard<std::mutex> lock(mutex_new_poly);
                if (_received_first_poly) {
                    _send_status_node_map();

                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    LOG_F(INFO, "Tracker checkNodeMap - process_stop=true; exiting");
    // hack wakeup condvar wait
    {
        std::lock_guard<std::mutex> lock(mutex_new_poly);
        new_poly=true;
    }
    cv_new_poly.notify_all();
}


void TrackingServer::_tr_update_poly() {
    std::unique_lock<std::mutex> lock(mutex_new_poly);

    while (! process_stop) {
        cv_new_poly.wait(lock, []{return new_poly;});
        LOG_F(WARNING, "Received new polygon");
        _received_first_poly = true;
        new_poly = false;
        if (! process_stop) {
            std::vector<Position> globalpoly = read_global_poly(FP_GLOBAL_AREA_POLYGON_PATH);
            {
                std::lock_guard<std::mutex> lock(_mutex_json_global_poly);
                std::string json_poly = get_json_of_poly(std::move(globalpoly));
                _json_global_poly = std::move(
                    string_to_chararray<TRACKING_GLOBALPOLY_MAXBUF>(std::move(json_poly))
                );
                ++_polyid;
            }
            _send_status_node_map();
        }
    }
    LOG_F(INFO, "TrackingServer update_poly - process_stop=true; exiting");
}


void TrackingServer::start() {
    LOG_F(WARNING, "Starting Tracking server");
    AbstractReliableBroadcastNode<TrackPacket>::start();

    _thread_check_node_map = std::thread(&TrackingServer::_tr_check_node_map, this);
    _thread_heartbeat = std::thread(&TrackingServer::_tr_heartbeat, this);
    _thread_update_poly = std::thread(&TrackingServer::_tr_update_poly, this);

    // send at least one status-nodemap anyway
    /*std::async(std::launch::async,
            [this] {
                std::this_thread::sleep_for(std::chrono::seconds(TRACKING_INITIAL_FP_SLEEP));
                send_status_node_map();
            }
        );*/
}

void TrackingServer::join() {
    AbstractReliableBroadcastNode<TrackPacket>::join();
    _thread_heartbeat.join();
    _thread_check_node_map.join();
    _thread_update_poly.join();
    LOG_F(WARNING, "TrServer: joined all threads");
}
