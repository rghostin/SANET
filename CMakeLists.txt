cmake_minimum_required(VERSION 3.15)
project(sanet)

# sudo dnf install libquadmath-devel -> Fedora 31

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-std=c++17 -Weffc++  -Wctor-dtor-privacy -masm=intel -ggdb3 -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion  -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept  -Woverloaded-virtual -Wsign-promo -Wzero-as-null-pointer-constant -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -fconcepts -mlong-double-128 -lquadmath -lpthread -ldl -lssl -lcrypto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

# OpenCV configuration
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Pour lancer les différents programmes
add_executable(robin robin.cpp)
add_executable(packet_broadcaster tools/tpacket_broadcaster.cpp)
add_executable(imagesender tools/imagesender.cpp)
add_executable(command_sender tools/command_sender.cpp)
add_executable(nodemap_sender tools/nodemap_sender.cpp)
add_executable(test_camera test_camera.cpp Camera.cpp Camera.hpp Position.hpp)

add_library(MyLib AbstractBroadcastNode.hpp AbstractReliableBroadcast.hpp common.hpp loguru.hpp loguru.cpp TrackPacket.hpp ImageChunkPacket.hpp Position.hpp TrackPacket.hpp ImageChunkPacket.hpp TrackingServer.hpp TrackingServer.cpp utils.hpp robin.cpp ImagingServer.cpp ImagingServer.hpp ImageBuilder.hpp Image.hpp ImageBuilder.cpp CCCommands.hpp CCServer.hpp CCServer.cpp)
add_library(LittleLib TrackPacket.hpp ImageChunkPacket.hpp settings.hpp Position.hpp robin.cpp common.hpp loguru.hpp utils.hpp loguru.cpp)

target_link_libraries(robin MyLib)
target_link_libraries(packet_broadcaster LittleLib)
target_link_libraries(imagesender LittleLib)
target_link_libraries(command_sender LittleLib)
target_link_libraries(nodemap_sender LittleLib)
target_link_libraries(test_camera ${OpenCV_LIBS})