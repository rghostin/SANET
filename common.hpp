#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <atomic>
#include <condition_variable>
#include <mutex>

/* Global to all threads of the process, initially set to false in main */
extern std::atomic<bool> process_stop;

extern std::mutex mutex_new_poly;
extern bool new_poly;
extern std::condition_variable cv_new_poly;

#endif