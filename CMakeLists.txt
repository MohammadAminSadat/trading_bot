cmake_minimum_required(VERSION 3.25)
project(TradingEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(FetchContent)
  
if (GTest_FOUND)
    message(STATUS "Using system Google Test")
    set(GTEST_LIBRARIES GTest::gtest GTest::gtest_main)
else()
    message(STATUS "System Google Test not found, will fetch from GitHub")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        # URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        # GIT_TAG v1.18.0
        URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.tar.gz
    )
    FetchContent_MakeAvailable(googletest)
    set(GTEST_LIBRARIES gtest gtest_main)
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.0)
  cmake_policy(SET CMP0167 OLD)
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.23)
  cmake_policy(SET CMP0144 OLD)
endif()

find_package(Boost REQUIRED)
enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(ClangFormat)

add_subdirectory(Modules)
add_subdirectory(CLI)
add_subdirectory(App)
