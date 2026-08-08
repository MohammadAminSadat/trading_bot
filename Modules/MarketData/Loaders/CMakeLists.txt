add_library(loader INTERFACE)
add_library(TradingEngine::MarketData::Loader ALIAS loader)
target_include_directories(loader INTERFACE inc)
target_link_libraries(loader INTERFACE TradingEngine::MarketData::Domain)
