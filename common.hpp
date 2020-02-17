#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <atomic>

/* Global to all threads of the process, initially set to false in main */
extern std::atomic<bool> process_stop;

#endif