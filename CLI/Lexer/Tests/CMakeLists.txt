add_executable(test_lexer test_lexer.cpp)
target_link_libraries(test_lexer PRIVATE TradingEngine::CLI::Lexer gtest_main gmock)
add_test(NAME CLI.Lexer.Lexer COMMAND test_lexer)

get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(CLI.Lexer.Lexer PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TOOLCHAIN_BIN}")
