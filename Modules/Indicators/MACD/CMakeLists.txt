add_library(MACD STATIC src/MACD.cpp)
add_library(TradingEngine::Indicators::MACD ALIAS MACD)
target_include_directories(MACD PUBLIC inc)
target_link_libraries(MACD PUBLIC TradingEngine::Indicators::Domain)