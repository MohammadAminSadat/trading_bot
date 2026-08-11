add_library(cli_domain INTERFACE)
add_library(TradingEngine::CLI::Domain ALIAS cli_domain)
target_include_directories(cli_domain INTERFACE inc)