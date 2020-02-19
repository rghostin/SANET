#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <atomic>
#include <condition_variable>
#include <mutex>

/* Global to all threads of the process, initially set to false in main */
extern std::atomic<bool> process_stop;
extern bool img_has_changed;
extern std::mutex mutex_img_has_changed;
extern std::condition_variable thread_cond_var;


#endif