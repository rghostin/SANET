#include "CCServer.hpp"



// helper function to add socket to array at correct position
bool add_socket(int* array, const size_t max_size, int newsockfd) {
    bool inserted=false;
    for (unsigned i=0; i<max_size; ++i) {
        if (array[i] == 0){
            array[i] = newsockfd;
            inserted=true;
            break;
        }
    }
    return inserted;    // false -> CC_MAX_CONNECTION exceeded
}


CCServer::CCServer(unsigned short port, uint8_t nodeID) :
_db(), _port(port), _nodeID(nodeID),
sockfd(), _srvaddr(), b_iface(), _thread_receiver()
{
    // Database setup
    _db = dbOpen(_path_db);

    // setup reception sockaddr
    memset(&_srvaddr, 0, sizeof(_srvaddr));
    _srvaddr.sin_family = AF_INET;
    _srvaddr.sin_addr.s_addr = INADDR_ANY;
    _srvaddr.sin_port = htons(_port);

}

CCServer::~CCServer() {
    if (_thread_receiver.joinable()) {
        _thread_receiver.join();
    }

    if (close(sockfd) < 0) {
        perror("Cannot close socket");
    }
    dbClose(_db);
}


void CCServer::_setup_socket() {
    // create socket
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Cannot create socket");
        throw;
    } 

    #ifdef __aarch64__
        LOG_F(WARNING, "ARM architecture detected");
        memset(&b_iface, 0, sizeof(b_iface));
        snprintf(b_iface.ifr_name, sizeof(b_iface.ifr_name), b_iface_name);
        if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void*)&b_iface, sizeof(b_iface)) < 0) {
            close(sockfd);
            perror("setsockopt (SO_BINDTODEVICE)");
            throw;
        }
    #else
     LOG_F(WARNING, "Standard architecture detected");
    #endif

    int opt=1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt (SO_REUSEADDR"); 
        throw; 
    } 

    // binding to port
    if ( bind(sockfd, reinterpret_cast<const struct sockaddr*>(&_srvaddr), sizeof(_srvaddr)) < 0 ) {
        close(sockfd);
        perror("Cannot bind");
        throw;
    }
    LOG_F(INFO, "Socket bound to port %d", _port);

    if (listen(sockfd, 3) < 0) { 
        perror("Cannot listen to port"); 
        throw; 
    } 
    LOG_F(INFO, "Socket listening to port %d", _port);
}



void CCServer::_tr_receiver() {
    fd_set readfds;
    timeval rcv_timeout = {1,0}; 
    int max_sd;
    long int nrecv;

    loguru::set_thread_name("CC:receiver");

    while (! process_stop) {
        int connsockfd;
        sockaddr_in cliaddr;
        socklen_t len_cliaddr;

        // initialize fdset
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        max_sd = sockfd;
        for (int i=0;i < CC_MAX_CONNECTIONS; ++i) {
            int sd = _online_sockets[i];
            if (sd > 0){
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // wait for activity
        if (select(max_sd+1, &readfds, nullptr, nullptr, &rcv_timeout) < 0) {
            perror("Cannot select");
            throw;
        }

        // process activity
        if ( FD_ISSET(sockfd, &readfds)) {
            // new connection
            if ( (connsockfd = accept(sockfd, reinterpret_cast<sockaddr*>(&cliaddr), &len_cliaddr)) < 0 ) {
                perror("Cannot accept connection");
                throw;
            }
            LOG_F(WARNING, "Connection from (%s, %d); connsockfd=%d", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), connsockfd);
            if (! add_socket(_online_sockets, CC_MAX_CONNECTIONS, connsockfd)) {
                LOG_F(ERROR, "Exceeded maximum number of allowed parallel connections : %d", CC_MAX_CONNECTIONS);
                continue;       // ignore
            }

        } else {
            // activity on other client connsockfd
            for (unsigned i=0; i<CC_MAX_CONNECTIONS; ++i) {
                int sd = _online_sockets[i];
                if (sd <= 0) {
                    continue;
                }

                if (FD_ISSET(sd, &readfds)) {
                    uint8_t command;
                    nrecv = recv(sd, &command, sizeof(command),0);      // todo ntoh, hton, ...
                    getpeername(sd , reinterpret_cast<sockaddr*>(&cliaddr) , &len_cliaddr); 

                    if ( nrecv == 0 ) {
                        // client hang up
                        LOG_F(WARNING, "Host disconnected , (%s:%d) \n" ,  inet_ntoa(cliaddr.sin_addr) , ntohs(cliaddr.sin_port));
                        _online_sockets[i] = 0; // reset to 0 in _online_sockets
                        if (close(sd) < 0){
                            perror("Cannot close socket");
                            throw;
                        }
                        
                    } else {
                        // data received
                        LOG_F(INFO, "received from (%s:%d) command=%d", inet_ntoa(cliaddr.sin_addr) , ntohs(cliaddr.sin_port), command);
                        std::async(std::launch::async, &CCServer::_dispatch, this, sd, command);
                    }
                }

            }
        }
    }
    LOG_F(INFO, "process_stop=true; exiting");
}


void CCServer::_dispatch(int socket, uint8_t command){

    switch (command) {
        case FETCH_NODES_POS:  // Fetch All positions
            _execute_fetch_all_pos(socket);
            break;
        case FETCH_GLOBAL_IMAGE:  // Fetch All Images
            break;
        case UPDATE_GLOBAL_AREA_POLYGON:  // Get updated area polygon
            _update_global_polygon(socket);
            break;
        default:
            LOG_F(WARNING, "Unknow command=%d", command);
            break;
    }
}


void CCServer::_execute_fetch_all_pos(int socket) {
    std::string map_node_json;
    map_node_json = dbFetchAllNodesPositions(_db);

    if (send(socket, map_node_json.c_str(), map_node_json.size(), 0) < 0) {
        perror("Cannot send the node map");
    }
 
    LOG_F(INFO, "Sent map : =%s", map_node_json.c_str());
}


void CCServer::_update_global_polygon(int socket) {
    FILE* polygon_file(fopen(FP_GLOBAL_AREA_POLYGON_PATH, "wb")); 

    char buffer[2048];
    memset(buffer, '\0', sizeof(buffer));

    if (recv(socket, &buffer, sizeof(buffer), 0) < 0) {
        perror("Cannot recv part of img");
        throw;
    }

    std::string json_encoded(buffer);
    std::stringstream ss(json_encoded);
    std::string temp;

    for (int i = 0; i < std::count(json_encoded.begin(), json_encoded.end(), '['); ++i) {
        std::getline(ss, temp, '[');
        std::getline(ss, temp, ']');
        temp += "\n";
        fwrite(temp.c_str(), sizeof(char), temp.size(), polygon_file);
    }

    fclose(polygon_file);
    {
        std::lock_guard<std::mutex> lock(mutex_new_poly);
        new_poly=true;
    }
    cv_new_poly.notify_one();
    LOG_F(WARNING, "Global area polygon file updated : %s", json_encoded.c_str());
}


void CCServer::start() {
    _setup_socket();
    _thread_receiver = std::thread(&CCServer::_tr_receiver, this);
}


void CCServer::join() {
    _thread_receiver.join();
}