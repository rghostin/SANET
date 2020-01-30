//#include <thread>
#include "Dispatcher.hpp"
#include "loguru.hpp"
#include "Tracker.hpp"


#define DISPATCHER_PORT 5820


int main(int argc, char** argv) {
    unsigned short nodeId(0);
    Tracker tracker;
    Dispatcher dispatcher(DISPATCHER_PORT, nodeId, tracker);

    loguru::init(argc, argv);
    loguru::add_file("robin.log", loguru::Append, loguru::Verbosity_WARNING);

    std::thread dispatch(&Dispatcher::start, &dispatcher);
    tracker.start();

    if (dispatch.joinable()) {
            dispatch.join();
    }

    return 0;
}