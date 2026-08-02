cmake_minimum_required(VERSION 3.22)
project(II LANGUAGES CXX)

find_package(Boost REQUIRED)

add_executable(out main.cpp)
target_link_libraries(out PRIVATE Boost::headers)

add_executable(beast_example beast_example.cpp)
target_link_libraries(beast_example PRIVATE Boost::headers)