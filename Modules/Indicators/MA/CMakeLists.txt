add_library(SMA STATIC src/SMA.cpp)
add_library(TradingEngine::Indicators::SMA ALIAS SMA)
target_include_directories(SMA PUBLIC inc)

target_link_libraries(SMA PUBLIC TradingEngine::Indicators::Domain Boost::headers)

add_library(EMA STATIC src/EMA.cpp)
add_library(TradingEngine::Indicators::EMA ALIAS EMA)
target_include_directories(EMA PUBLIC inc)

target_link_libraries(EMA PUBLIC TradingEngine::Indicators::Domain)
