cmake_minimum_required(VERSION 3.22)
project(splitter)

set(CMAKE_CXX_STANDARD 17)

add_executable(splitter main.cpp)

find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(splitter cxxopts::cxxopts)