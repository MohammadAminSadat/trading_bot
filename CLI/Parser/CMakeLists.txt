add_library(command_parser STATIC src/CommandParser.cpp)
add_library(TradingEngine::CLI::CommandParser ALIAS command_parser)
target_include_directories(command_parser PUBLIC inc)
target_link_libraries(command_parser PRIVATE TradingEngine::CLI::Domain)