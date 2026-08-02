add_library(IndicatorsCore STATIC src/indicators.cpp)
add_library(TradingEngine::Indicators::Core ALIAS IndicatorsCore)
target_include_directories(IndicatorsCore PUBLIC inc)