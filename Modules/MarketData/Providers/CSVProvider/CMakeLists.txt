add_library(CSVProvider STATIC src/CSVProvider.cpp)
add_library(TradingEngine::MarketData::CSVProvider ALIAS CSVProvider)
target_include_directories(CSVProvider PUBLIC inc)
target_link_libraries(CSVProvider PUBLIC TradingEngine::MarketData::Core 
                                         TradingEngine::Core::CSV 
                                         TradingEngine::Core::Core)