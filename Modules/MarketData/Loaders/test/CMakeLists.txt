add_executable(test_loader test_loader.cpp)
target_link_libraries(test_loader PRIVATE TradingEngine::MarketData::Loader
                                          TradingEngine::MarketData::CSVProvider
                                          gtest_main gmock)
add_test(NAME MarketData.Loaders COMMAND test_loader)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(MarketData.Loaders PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")