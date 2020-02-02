#include <thread>
#include <atomic>
#include <csignal>
#include "loguru.hpp"
#include "common.hpp"
#include "MainServer.hpp"
#include "ReliableMainServer.hpp"
#include "Tracker.hpp"


#define HEARTBEAT_PERIOD 30
#define PEER_LOSS_TIMEOUT 3 * HEARTBEAT_PERIOD
#define RELIABLE_PACKET_MAX_AGE 2*HEARTBEAT_PERIOD // heartbeat < max_packet_age < lost_peer_timeout
#define PERIOD_CHECK_NODEMAP 30
#define MAINSERVER_PORT 5820



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
    Tracker tracker(PEER_LOSS_TIMEOUT, PERIOD_CHECK_NODEMAP);
    //MainServer mainserver(MAINSERVER_PORT, nodeID, tracker, HEARTBEAT_PERIOD);
    ReliableMainServer relmainserver(MAINSERVER_PORT, nodeID, tracker, HEARTBEAT_PERIOD, RELIABLE_PACKET_MAX_AGE);

    // setup signals
    signal(SIGINT, exit_handler);

    // setup logs - TODO -v for verbosity
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
    loguru::add_file("robin.log", loguru::Append, loguru::Verbosity_WARNING);
    loguru::set_thread_name("robin main");

    // start all services on parallel threads
    std::thread thread_tracker(&Tracker::start, &tracker);

    // start server on current thread
    //mainserver.start();
    relmainserver.start();

    // join all services
    relmainserver.join();
    //mainserver.join();
    thread_tracker.join();

    LOG_F(WARNING, "Robin exiting");
    return 0;
}