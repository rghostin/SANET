//#include <thread>
#include "Dispatcher.hpp"
#include "loguru.hpp"


#define DISPATCHER_PORT 5820


int main(int argc, char** argv) {
    Dispatcher dispatcher(DISPATCHER_PORT);

    loguru::init(argc, argv);
    loguru::add_file("robin.log", loguru::Append, loguru::Verbosity_WARNING);

    dispatcher.start();
    return 0;
}