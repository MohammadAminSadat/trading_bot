cmake_minimum_required(VERSION 3.25)
project(TradingEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        79524dd
)
  
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        52eb810
)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG        bc3b3987
)
      
FetchContent_MakeAvailable(spdlog googletest eigen)
if(WIN32)
  cmake_policy(SET CMP0167 OLD)
else()
  cmake_policy(SET CMP0144 OLD)
endif()
find_package(Boost REQUIRED)
add_subdirectory(Modules)
add_subdirectory(App)
