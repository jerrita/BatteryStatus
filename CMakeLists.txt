cmake_minimum_required(VERSION 3.15)

project(BatteryMonitor)

set(CMAKE_CXX_STANDARD 17)

include_directories(./include)
add_executable(BatteryMonitor WIN32 src/main.cpp src/battery.cpp)