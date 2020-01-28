cmake_minimum_required(VERSION 3.15)
project(tracker)

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-std=c++17 -fconcepts -mlong-double-128 -ggdb3 -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion -Weffc++ -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept -Wctor-dtor-privacy -Wsign-promo -Wzero-as-null-pointer-constant -pthread -pedantic -lpthread -ldl")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

add_executable(tracker Tracker.cpp Tracker.hpp Dispatcher.cpp Dispatcher.hpp loguru.cpp loguru.hpp packets.hpp robin.cpp)