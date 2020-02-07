#include <thread>
#include <atomic>
#include <csignal>
#include "loguru.hpp"
#include "utils.hpp"
#include "settings.hpp"
#include "common.hpp"
#include "TrackingServer.hpp"
#include "Tracker.hpp" 

//#include "utils_log.hpp"

// TODO: getopt, nodeID mandatory, -v optional default 0


// Stopping mechanism 
std::atomic<bool> process_stop(false);

void exit_handler(int s) {
    LOG_F(WARNING, "Caught signal %d", s);
    process_stop = true;
    LOG_F(INFO, "program_stop=true"); 
}


int main(int argc, char** argv) {
    uint8_t nodeID; // = read_int_from_file(CFG_NODEID_FNAME);
    LOG_F(INFO, "CFG: NodeID=%d", nodeID);
    Tracker tracker(TRACKING_PEER_LOSS_TIMEOUT, TRACKING_PERIOD_CHECK_NODEMAP);
    TrackingServer trackingserver(TRACKING_SERVER_PORT, nodeID, tracker, TRACKING_HEARTBEAT_PERIOD);

    // setup signals
    signal(SIGINT, exit_handler);

    // setup logs - TODO -v for verbosity
    loguru::g_stderr_verbosity = loguru::Verbosity_3;
    loguru::add_file("robin.log", loguru::Append, loguru::Verbosity_INFO);
    loguru::set_thread_name("robin_main");


    if (argc > 2) {
        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], "-v") == 0 and i + 1 < argc) {
                char *checkLong;
                long int verbose_value(strtol (argv[i + 1], &checkLong, 10));

                if (std::strcmp(checkLong, "\0") == 0 and (-2 <= verbose_value) and (verbose_value <= 9)) {
                    loguru::g_stderr_verbosity = static_cast<int>(verbose_value);
                    LOG_F(WARNING, "Logging changed to the value -> {%d} through parameter -v", loguru::g_stderr_verbosity);
                }
            } else if (std::strcmp(argv[i], "--nodeid") == 0 and i + 1 < argc) {
                char *checkLong;
                long int verbose_value(strtol (argv[i + 1], &checkLong, 10));

                if (std::strcmp(checkLong, "\0") == 0 and (0 <= verbose_value) and (verbose_value <= 100)) {
                    nodeID = static_cast<int>(verbose_value);
                    LOG_F(INFO, "NodeID changed to the value -> {%d} through parameter -v", loguru::g_stderr_verbosity);
                }


            }
        }
    }
    

    // start server on current thread
    trackingserver.start();

    // join all services
    trackingserver.join();

    LOG_F(WARNING, "Robin exiting");
    return 0;
}
