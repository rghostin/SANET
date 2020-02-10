cmake_minimum_required(VERSION 3.15)
project(tracker)

# sudo dnf install libquadmath-devel -> Fedora 31

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-std=c++17 -Weffc++  -Wctor-dtor-privacy -masm=intel -ggdb3 -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion  -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept  -Woverloaded-virtual -Wsign-promo -Wzero-as-null-pointer-constant -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -fconcepts -mlong-double-128 -lquadmath -lpthread -ldl")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

# Pour lancer les différents programmes
add_executable(robin robin.cpp)
add_executable(packetsender packetsender.cpp)
add_library(MyLib AbstractBroadcastNode.hpp AbstractReliableBroadcast.hpp common.hpp loguru.hpp loguru.cpp packets.hpp Position.hpp Tracker.hpp Tracker.cpp TrackingServer.hpp TrackingServer.cpp utils.hpp robin.cpp ImagingServer.cpp ImagingServer.hpp Image.hpp)

target_link_libraries(robin MyLib)
target_link_libraries(packetsender MyLib)


# ATTENTION : Pour faire tourner robin, il faut placer le nodeid.cfg dans cmake-build-debug !