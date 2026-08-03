cmake_minimum_required(VERSION 3.22)
project(II LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS filesystem)

add_executable(out main.cpp)
target_link_libraries(out PRIVATE Boost::headers)

add_executable(beast_example beast_example.cpp)
target_link_libraries(beast_example PRIVATE Boost::headers)

add_executable(flyweight_example flyweight_example.cpp)
target_link_libraries(flyweight_example PRIVATE Boost::headers)

add_library(plugin SHARED plugin_lib.cpp)

add_executable(dll_crc_example dll_crc_example.cpp)
target_link_libraries(dll_crc_example PRIVATE Boost::headers Boost::filesystem ${CMAKE_DL_LIBS})
set_target_properties(dll_crc_example PROPERTIES
    BUILD_RPATH "${Boost_LIBRARY_DIRS}"
)