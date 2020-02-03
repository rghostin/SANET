#include <thread>
#include <atomic>
#include <csignal>
#include "loguru.hpp"
#include "settings.hpp"
#include "common.hpp"
#include "TrackingServer.hpp"
#include "Tracker.hpp"

// TODO: maxpacket_age param
// tracket intenal


/* Stopping mechanism -start */

std::atomic<bool> process_stop(false);

void exit_handler(int s) {
    LOG_F(WARNING, "Caught signal %d", s);
    process_stop = true;
    LOG_F(INFO, "program_stop=true"); 
}
/* Stopping mechanism - end */



int main(int argc, char** argv) {
    uint8_t nodeID = 0;
    Tracker tracker(TRACKING_PEER_LOSS_TIMEOUT, TRACKING_PERIOD_CHECK_NODEMAP);
    TrackingServer trackingserver(TRACKING_SERVER_PORT, nodeID, tracker, TRACKING_HEARTBEAT_PERIOD);

    // setup signals
    //signal(SIGINT, exit_handler);

    // setup logs - TODO -v for verbosity
    loguru::g_stderr_verbosity = loguru::Verbosity_3;
    loguru::add_file("robin.log", loguru::Append, loguru::Verbosity_WARNING);
    loguru::set_thread_name("robin_main");

    // start all services on parallel threads
    std::thread thread_tracker(&Tracker::start, &tracker);

    // start server on current thread
    //mainserver.start();
    trackingserver.start();

    // join all services
    trackingserver.join();
    thread_tracker.join();

    LOG_F(WARNING, "Robin exiting");
    return 0;
}