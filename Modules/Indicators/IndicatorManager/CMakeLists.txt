add_library(indicator_manager STATIC src/IndicatorManager.cpp)
add_library(TradingEngine::Indicators::Orchestrator ALIAS indicator_manager)
target_include_directories(indicator_manager PUBLIC inc)
target_link_libraries(indicator_manager PUBLIC TradingEngine::Indicators::Domain
                                                    TradingEngine::MarketData::Domain)

add_subdirectory(Tests)
