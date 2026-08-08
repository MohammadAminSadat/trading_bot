add_executable(test_csvprovider test_csvprovider.cpp)
target_link_libraries(test_csvprovider PRIVATE TradingEngine::MarketData::CSVProvider gtest_main gmock)
add_test(NAME MarketData.Providers.CSVProvider COMMAND test_csvprovider)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(MarketData.Providers.CSVProvider PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")