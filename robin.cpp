//#include <thread>
#include "Dispatcher.hpp"
#include "loguru.hpp"
#include "Tracker.hpp"

#define HEARTBEAT 30
#define TIMEOUT 3 * HEARTBEAT
#define INTERVALTIME_CHECKTIMESTAMP 30
#define DISPATCHER_PORT 5820


int main(int argc, char** argv) {
    uint8_t nodeId(0);
    Tracker tracker(TIMEOUT, INTERVALTIME_CHECKTIMESTAMP);
    Dispatcher dispatcher(DISPATCHER_PORT, nodeId, tracker, HEARTBEAT);

    loguru::init(argc, argv);
    loguru::add_file("robin.log", loguru::Append, loguru::Verbosity_WARNING);

    std::thread dispatch(&Dispatcher::start, &dispatcher);
    tracker.start();

    if (dispatch.joinable()) {
            dispatch.join();
    }

    return 0;
}