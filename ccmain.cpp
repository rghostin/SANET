#include <csignal>
#include "CCServer.hpp"
#include "loguru.hpp"
#include "utils.hpp"
#include "settings.hpp"
#include "common.hpp"


std::atomic<bool> process_stop(false);

void exit_handler(int s) {
    LOG_F(WARNING, "Caught signal %d", s);
    process_stop = true;
    LOG_F(INFO, "program_stop=true"); 
}



int main() {
    const uint8_t nodeID = read_int_from_file(CFG_NODEID);

    // setup signals 
    signal(SIGINT, exit_handler);


     // setup logs 
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
    loguru::add_file("ccmain.log", loguru::Truncate, loguru::Verbosity_3);
    loguru::set_thread_name("cc_main");



    sqlite3_config(SQLITE_CONFIG_SERIALIZED);


    CCServer ccserver(CC_SERVER_PORT, nodeID);

    ccserver.start();
    ccserver.join();
    return 0;
}