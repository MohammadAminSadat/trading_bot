add_library(indicator_domain STATIC src/indicators.cpp)
add_library(TradingEngine::Indicators::Domain ALIAS indicator_domain)
target_include_directories(indicator_domain PUBLIC inc)