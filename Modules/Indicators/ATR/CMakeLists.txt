add_library(ATR STATIC src/atr.cpp)
add_library(TradingEngine::Indicators::ATR ALIAS ATR)
target_include_directories(ATR PUBLIC inc)
target_link_libraries(ATR PUBLIC TradingEngine::Indicators::Core)