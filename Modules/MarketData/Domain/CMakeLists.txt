add_library(market_data_domain INTERFACE)
add_library(TradingEngine::MarketData::Domain ALIAS market_data_domain)
target_include_directories(market_data_domain INTERFACE inc)
target_link_libraries(market_data_domain INTERFACE TradingEngine::Core::Core)

add_subdirectory(test)