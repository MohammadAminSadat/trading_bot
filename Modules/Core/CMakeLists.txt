add_library(Core STATIC src/Core.cpp)
target_include_directories(Core PUBLIC inc)
add_library(TradingEngine::Core::Core ALIAS Core)

add_subdirectory(CSV)