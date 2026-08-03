add_library(candle src/candle.cpp)
add_library(TradingEngine::MarketData::candle ALIAS candle)
target_include_directories(candle PUBLIC inc)