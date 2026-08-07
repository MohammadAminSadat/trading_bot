add_library(market_data_core INTERFACE)
add_library(TradingEngine::MarketData::Core ALIAS market_data_core)
target_include_directories(market_data_core INTERFACE inc)
target_link_libraries(market_data_core INTERFACE TradingEngine::Core::Core)

add_subdirectory(test)