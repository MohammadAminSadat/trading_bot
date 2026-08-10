add_executable(test_manager test_manager.cpp)
target_link_libraries(test_manager PRIVATE TradingEngine::Indicators::Orchestrator
                                         TradingEngine::Indicators::SMA
                                         TradingEngine::Indicators::EMA
                                         TradingEngine::Indicators::ATR
                                         gtest_main gmock)
add_test(NAME Indicators.IndicatorManager.Manager COMMAND test_manager)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(Indicators.IndicatorManager.Manager PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")
