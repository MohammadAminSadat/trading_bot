add_library(RSI STATIC src/RSI.cpp src/ExponentialRSI.cpp src/WilderSmoothingRSI.cpp)
add_library(TradingEngine::Indicators::RSI ALIAS RSI)
target_include_directories(RSI PUBLIC inc)
target_link_libraries(RSI PUBLIC TradingEngine::Indicators::Domain Boost::headers)

add_subdirectory(Tests)