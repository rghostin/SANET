#include <thread>
#include "loguru.hpp"
#include "MainServer.hpp"
#include "ReliableMainServer.hpp"
#include "Tracker.hpp"


#define HEARTBEAT_PERIOD 30
#define PEER_LOSS_TIMEOUT 3 * HEARTBEAT_PERIOD
#define RELIABLE_PACKET_MAX_AGE 2*HEARTBEAT_PERIOD // heartbeat < max_packet_age < lost_peer_timeout
#define PERIOD_CHECK_NODEMAP 30
#define MAINSERVER_PORT 5820


int main(int argc, char** argv) {
    uint8_t nodeID = 0;
    Tracker tracker(PEER_LOSS_TIMEOUT, PERIOD_CHECK_NODEMAP);
    int i=1;
    MainServer mainServer;

    while(i<argc) {
        if (argv[i] == "--reliable-broadcast") {
            mainServer = ReliableMainServer(MAINSERVER_PORT, nodeID, tracker, HEARTBEAT_PERIOD, RELIABLE_PACKET_MAX_AGE);
        } else if (argv[i] == "--unreliable-broadcast") {
            mainServer = MainServer(MAINSERVER_PORT, nodeID, tracker, HEARTBEAT_PERIOD);
        } else if (argv[i] == "-v" ){
            loguru::g_stderr_verbosity (default: loguru::Verbosity_INFO);
        }
        ++i;
    }

    loguru::init(argc, argv);
    loguru::add_file("robin.log", loguru::Append, loguru::Verbosity_WARNING);
    loguru::set_thread_name("main");

    // start all services on parallel threads
    std::thread thread_tracker(&Tracker::start, &tracker);

    if(param_broadcast==0 or param_broadcast==1){
        // start server on current thread
        reliableMainServer.start();
        // join all services
        reliableMainServer.join();
    }
    else {
        // start server on current thread
        mainServer.start();
        // join all services
        mainServer.join();
    }
    thread_tracker.join();

    return 0;
}