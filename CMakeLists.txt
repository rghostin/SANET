cmake_minimum_required(VERSION 3.15)
project(tracker)

# sudo dnf install libquadmath-devel -> Fedora 31

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-std=c++17 -Weffc++  -Wctor-dtor-privacy -masm=intel -ggdb3 -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion  -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept  -Woverloaded-virtual -Wsign-promo -Wzero-as-null-pointer-constant -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -fconcepts -mlong-double-128 -lquadmath -lpthread -ldl -lssl -lcrypto -lsqlite3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Pour lancer les différents programmes
add_executable(robin robin.cpp)
add_executable(ccobin CCobin.cpp)
add_executable(cmd_sender tools/command_sender.cpp)
add_executable(img_sender tools/imagesender.cpp)
add_executable(nd_sender tools/nodemap_sender.cpp)
add_executable(packt_sender tools/tpacket_broadcaster.cpp)

add_library(ABNLibs AbstractBroadcastNode.hpp AbstractReliableBroadcast.hpp common.hpp loguru.hpp loguru.cpp TrackPacket.hpp ImageChunkPacket.hpp Position.hpp TrackingServer.hpp TrackingServer.cpp utils.hpp ImagingServer.cpp ImagingServer.hpp ImageBuilder.hpp Image.hpp ImageBuilder.cpp database_utils/DbUtils.hpp CCPackets.hpp)
add_library(ToolsLib TrackPacket.hpp ImageChunkPacket.hpp settings.hpp Position.hpp robin.cpp common.hpp loguru.hpp utils.hpp loguru.cpp)
add_library(CCLib TrackPacket.hpp CCServer.hpp CCServer.cpp loguru.hpp loguru.cpp common.hpp settings.hpp database_utils/DbUtils.hpp CCobin.cpp CCCommands.hpp CCPackets.hpp)

target_link_libraries(robin ABNLibs)
target_link_libraries(ccobin CCLib)
target_link_libraries(cmd_sender ToolsLib)
target_link_libraries(img_sender ToolsLib)
target_link_libraries(nd_sender ToolsLib)
target_link_libraries(packt_sender ToolsLib)