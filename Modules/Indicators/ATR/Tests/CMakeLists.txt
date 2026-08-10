add_executable(test_atr test_atr.cpp)
target_link_libraries(test_atr PRIVATE TradingEngine::Indicators::ATR gtest_main gmock)
add_test(NAME Indicators.ATR.ATR COMMAND test_atr)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(Indicators.ATR.ATR PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")
