add_library(MA STATIC src/MA.cpp)
add_library(TradingEngine::Indicators::MA ALIAS MA)
target_include_directories(MA PUBLIC inc)
target_link_libraries(MA PUBLIC TradingEngine::Indicators::Domain)