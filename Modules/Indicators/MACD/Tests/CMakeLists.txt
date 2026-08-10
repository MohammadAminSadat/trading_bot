add_executable(test_macd test_macd.cpp)
target_link_libraries(test_macd PRIVATE TradingEngine::Indicators::MACD gtest_main gmock)
add_test(NAME Indicators.MACD.MACD COMMAND test_macd)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(Indicators.MACD.MACD PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")
