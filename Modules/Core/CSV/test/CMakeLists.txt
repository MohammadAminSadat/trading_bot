add_executable(test_csv test_csv.cpp)
target_link_libraries(test_csv PRIVATE TradingEngine::Core::CSV gtest_main gmock)
add_test(NAME Core.CSV.Parser COMMAND test_csv)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(Core.CSV.Parser PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")
