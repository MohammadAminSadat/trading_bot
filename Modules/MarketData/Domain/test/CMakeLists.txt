add_executable(test_candle test_candle.cpp)
target_link_libraries(test_candle PRIVATE TradingEngine::MarketData::Domain gtest_main gmock)
add_test(NAME MarketData.Core.Candle COMMAND test_candle)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(MarketData.Core.Candle PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")
