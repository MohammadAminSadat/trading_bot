add_library(indicator_domain INTERFACE)
add_library(TradingEngine::Indicators::Domain ALIAS indicator_domain)
target_include_directories(indicator_domain INTERFACE inc)
target_link_libraries(indicator_domain INTERFACE TradingEngine::MarketData::Domain)