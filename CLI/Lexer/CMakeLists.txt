add_library(lexer STATIC src/Lexer.cpp)
add_library(TradingEngine::CLI::Lexer ALIAS lexer)
target_include_directories(lexer PUBLIC inc)
target_link_libraries(lexer PUBLIC TradingEngine::CLI::Domain)

add_subdirectory(Tests)