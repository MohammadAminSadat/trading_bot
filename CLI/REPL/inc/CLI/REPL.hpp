#pragma once
#include <CLI/Lexer.hpp>
#include <iostream>
#include <string>

namespace TradingEngine::CLI {
class REPL {
public:
  REPL() = default;
  REPL(std::string);
  int run();

private:
  void initialize();
  void print_help();
  void clean();
  const Lexer lexer_{};
  std::string application_name_{"TradingEngine"};
};

} // namespace TradingEngine::CLI