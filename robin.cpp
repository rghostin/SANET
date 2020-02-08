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

    uint8_t nodeID(255); // = read_int_from_file(CFG_NODEID_FNAME);
    int opt;
    char *checkLong;
    long int verbose_value;

#ifdef __aarch64__
    auto user_running(getuid());
    if (user_running != 0) {
        perror("Robin must be run as root");
        throw;
    }
#endif

    if (argc < 3) {
        perror("Not enough arguments");
        throw;
    }

    // setup logs - TODO -v for verbosity
    loguru::g_stderr_verbosity = loguru::Verbosity_3;
    loguru::add_file("robin.log", loguru::Truncate, loguru::Verbosity_7);
    loguru::set_thread_name("robin_main");


    while((opt = getopt(argc, argv, "i:v:")) != -1)
    {
        switch(opt)
        {
            case 'i':
                verbose_value = strtol (optarg, &checkLong, 10);

                if (std::strcmp(checkLong, "\0") == 0 and (0 <= verbose_value) and (verbose_value <= 254)) {
                    nodeID = static_cast<uint8_t>(verbose_value);
                }
                else {
                    perror("[NodeID] - OPTARG not an integer or not in range [0:100] !");
                    throw;
                }

                LOG_F(WARNING, "NodeID=%d", nodeID);

                break;
            case 'v':
                verbose_value = strtol (optarg, &checkLong, 10);

                if (std::strcmp(checkLong, "\0") == 0 and (-2 <= verbose_value) and (verbose_value <= 9)) {
                    loguru::g_stderr_verbosity = static_cast<int>(verbose_value);
                    LOG_F(WARNING, "Logging changed to the value -> {%d} through parameter -v", loguru::g_stderr_verbosity);
                }

                break;
        }
    }

    if (nodeID == 255) {
        perror("NodeID not fixed !");
        throw;
    }


    Tracker tracker(TRACKING_PEER_LOSS_TIMEOUT, TRACKING_PERIOD_CHECK_NODEMAP);
    TrackingServer trackingserver(TRACKING_SERVER_PORT, nodeID, tracker, TRACKING_HEARTBEAT_PERIOD);

    // setup signals
    signal(SIGINT, exit_handler);


    // start server on current thread
    trackingserver.start();

    // join all services
    trackingserver.join();

    LOG_F(WARNING, "Robin exiting");
    return 0;
}
