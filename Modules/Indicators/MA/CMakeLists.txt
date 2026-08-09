add_library(SMA STATIC src/SMA.cpp)
add_library(TradingEngine::Indicators::SMA ALIAS SMA)
target_include_directories(SMA PUBLIC inc)

target_link_libraries(SMA PUBLIC TradingEngine::Indicators::Domain Boost::headers) 