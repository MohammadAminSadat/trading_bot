cmake_minimum_required(VERSION 3.25)
project(TradingEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(FetchContent)
  
FetchContent_Declare(
  googletest
  # GIT_REPOSITORY https://github.com/google/googletest.git
  # GIT_TAG        52eb810
  URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.tar.gz
)
      
FetchContent_MakeAvailable(googletest)
if(WIN32)
  cmake_policy(SET CMP0167 OLD)
else()
  cmake_policy(SET CMP0144 OLD)
endif()
find_package(Boost REQUIRED)
enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(ClangFormat)

add_subdirectory(Modules)
add_subdirectory(App)
