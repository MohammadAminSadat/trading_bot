#include <CLI/REPL.hpp>
#include <iostream>

int main() {
  TradingEngine::CLI::REPL application("TradingEngine");
  return application.run();
}