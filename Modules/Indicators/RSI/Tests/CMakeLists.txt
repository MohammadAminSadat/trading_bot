add_executable(test_rsi test_rsi.cpp)
target_link_libraries(test_rsi PRIVATE TradingEngine::Indicators::RSI gtest_main gmock)
add_test(NAME Indicators.RSI.RSI COMMAND test_rsi)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(Indicators.RSI.RSI PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")
