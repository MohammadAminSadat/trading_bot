add_executable(test_candle test_candle.cpp)
target_link_libraries(test_candle PRIVATE TradingEngine::MarketData::Core gtest_main gmock)
add_test(NAME MarketData.Core.Candle COMMAND test_candle)
