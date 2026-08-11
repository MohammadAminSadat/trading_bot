add_library(REPL STATIC src/REPL.cpp)
add_library(TradingEngine::CLI::REPL ALIAS REPL)
target_include_directories(REPL PUBLIC inc)
target_link_libraries(REPL PUBLIC TradingEngine::CLI::Domain TradingEngine::CLI::Lexer)
