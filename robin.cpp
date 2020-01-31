#include <thread>
#include "loguru.hpp"
#include "MainServer.hpp"
#include "Tracker.hpp"


#define HEARTBEAT_PERIOD 30
#define PEER_LOSS_TIMEOUT 3 * HEARTBEAT_PERIOD
#define PERIOD_CHECK_NODEMAP 30
#define MAINSERVER_PORT 5820


int main(int argc, char** argv) {
    uint8_t nodeID = 0;
    Tracker tracker(PEER_LOSS_TIMEOUT, PERIOD_CHECK_NODEMAP);
    MainServer mainserver(MAINSERVER_PORT, nodeID, tracker, HEARTBEAT_PERIOD);

    loguru::init(argc, argv);
    loguru::add_file("robin.log", loguru::Append, loguru::Verbosity_WARNING);
    loguru::set_thread_name("main");

    // start all services on parallel threads
    std::thread thread_tracker(&Tracker::start, &tracker);

    // start server on current thread
    mainserver.start();

    // join all services
    thread_tracker.join();

    return 0;
}