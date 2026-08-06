add_library(CSVImporter STATIC src/CSVImporter.cpp)
add_library(TradingEngine::MarketData::CSVImporter ALIAS CSVImporter)
target_include_directories(CSVImporter PUBLIC inc)
target_link_libraries(CSVImporter PUBLIC    TradingEngine::MarketData::candle 
                                            TradingEngine::Core::CSV 
                                            TradingEngine::Core::Core)