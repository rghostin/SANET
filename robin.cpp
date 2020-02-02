#include <thread>
#include "loguru.hpp"
#include "MainServer.hpp"
#include "ReliableMainServer.hpp"
#include "Tracker.hpp"


#define HEARTBEAT_PERIOD 30
#define PEER_LOSS_TIMEOUT 3 * HEARTBEAT_PERIOD
#define RELIABLE_PACKET_MAX_AGE 2 * HEARTBEAT_PERIOD // heartbeat < max_packet_age < lost_peer_timeout
#define PERIOD_CHECK_NODEMAP 30
#define MAINSERVER_PORT 5820


int main(int argc, char** argv) {
    uint8_t nodeID = 0;
    Tracker tracker(PEER_LOSS_TIMEOUT, PERIOD_CHECK_NODEMAP);
    MainServer *mainServer(nullptr);
    bool res_comparaison_argv(false);

    //    loguru::init(argc, argv);
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
    loguru::add_file("robin.log", loguru::Append, loguru::Verbosity_WARNING);
    loguru::set_thread_name("main");

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], "--unreliable-broadcast") == 0) {
                res_comparaison_argv = true;
            }
            else if (std::strcmp(argv[i], "-v") == 0 and i + 1 < argc) {
                char *checkLong;
                long int verbose_value(strtol (argv[i + 1], &checkLong, 10));

                if (std::strcmp(checkLong, "\0") == 0 and (-2 <= verbose_value) and (verbose_value <= 9)) {
                    loguru::g_stderr_verbosity = static_cast<int>(verbose_value);
                }
            }
        }
    }

    if (res_comparaison_argv) {
        mainServer = new MainServer(MAINSERVER_PORT, nodeID, tracker, HEARTBEAT_PERIOD);
    }
    else {
        mainServer = new ReliableMainServer(MAINSERVER_PORT, nodeID, tracker, HEARTBEAT_PERIOD, RELIABLE_PACKET_MAX_AGE);
    }

    // start all services on parallel threads
    std::thread thread_tracker(&Tracker::start, &tracker);

    // start server on current thread
    mainServer->start();
    // join all services
    mainServer->join();

    thread_tracker.join();

    delete mainServer;
    mainServer = nullptr;

    return 0;
}