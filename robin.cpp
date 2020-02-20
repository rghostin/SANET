#include <thread>
#include <atomic>
#include <csignal>
#include "loguru.hpp"
#include "utils.hpp"
#include "settings.hpp"
#include "common.hpp"
//#include "TrackingServer.hpp"
#include "ImagingServer.hpp"
#include "CCServer.hpp"
#include <sqlite3.h>
#include <condition_variable>


// Stopping mechanism 
std::atomic<bool> process_stop(false);
bool img_has_changed(false);
std::mutex mutex_img_has_changed;
std::condition_variable thread_cond_var;


void exit_handler(int s) {
    LOG_F(WARNING, "Caught signal %d", s);
    process_stop = true;
    LOG_F(INFO, "program_stop=true"); 
}


int main(int argc, char** argv) {

    const uint8_t nodeID = read_int_from_file(CFG_NODEID);
    int opt;
    char *checkLong;
    long int verbose_value;

    // force running as root on ARM
    #ifdef __aarch64__
        auto user_running = getuid();
        if (user_running != 0) {
            perror("Robin must be run as root");
            throw;
        }
    #endif

    if (argc < 2) {
        std::cerr << "Not enough arguments" << std::endl;
        throw;
    }


    // setup logs 
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
    loguru::add_file("robin.log", loguru::Truncate, loguru::Verbosity_3);
    loguru::set_thread_name("robin_main");
    LOG_F(WARNING, "NodeID=%d", nodeID);

    while((opt = getopt(argc, argv, "v:")) != -1) {
        switch(opt)
        {
            case 'v':
                verbose_value = strtol (optarg, &checkLong, 10);
                if (std::strcmp(checkLong, "\0") == 0 and (-2 <= verbose_value) and (verbose_value <= 9)) {
                    loguru::g_stderr_verbosity = static_cast<int>(verbose_value);
                    LOG_F(WARNING, "Logging changed to the value -> {%d} through parameter -v", loguru::g_stderr_verbosity);
                }
                break;
        }
    }

    // setup signals
    signal(SIGINT, exit_handler);

    sqlite3_config(SQLITE_CONFIG_SERIALIZED);

    // Tracking server  //TODO
//    TrackingServer trackingServer(TRACKING_SERVER_PORT, nodeID);
//    trackingServer.start();

    // Imaging server
    ImagingServer imagingServer(IMAGING_SERVER_PORT, nodeID);
    imagingServer.start();

    // CC server
    CCServer ccServer(CC_SERVER_PORT, nodeID);
    ccServer.start();

    // join all services
    ccServer.join();
//    trackingServer.join();
    imagingServer.join();

    LOG_F(WARNING, "Robin exiting");
    return 0;
}
