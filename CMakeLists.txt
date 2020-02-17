cmake_minimum_required(VERSION 3.10)
project(sanet)

# sudo dnf install libquadmath-devel -> Fedora 31

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-std=c++17 -Weffc++  -Wctor-dtor-privacy -masm=intel -ggdb3 -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion  -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept  -Woverloaded-virtual -Wsign-promo -Wzero-as-null-pointer-constant -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -fconcepts -mlong-double-128 -lquadmath -lpthread -ldl -lssl -lcrypto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

# OpenCV configuration
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Pour lancer les différents programmes

add_executable(test_camera test_camera.cpp Camera.cpp Camera.hpp Position.hpp)


target_link_libraries(test_camera ${OpenCV_LIBS})