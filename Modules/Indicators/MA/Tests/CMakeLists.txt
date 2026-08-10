add_executable(test_sma test_sma.cpp)
target_link_libraries(test_sma PRIVATE TradingEngine::Indicators::SMA gtest_main gmock)
add_test(NAME Indicators.MA.SMA COMMAND test_sma)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(Indicators.MA.SMA PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")

add_executable(test_ema test_ema.cpp)
target_link_libraries(test_ema PRIVATE TradingEngine::Indicators::EMA gtest_main gmock)
add_test(NAME Indicators.MA.EMA COMMAND test_ema)

set_tests_properties(Indicators.MA.EMA PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")
