#pragma once
#include <iostream>
#include <string>

namespace TradingEngine::CLI {
class REPL {
public:
  REPL() = default;
  REPL(std::string);
  int run();
  ~REPL() = default;

private:
  void initialize();
  void print_help();
  void clean();
  std::string application_name_{"Trading_Engine"};
};

} // namespace TradingEngine::CLI