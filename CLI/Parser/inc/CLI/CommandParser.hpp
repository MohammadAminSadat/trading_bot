#pragma once
#include <CLI/Command.hpp>
#include <CLI/Token.hpp>
#include <vector>
namespace TradingEngine::CLI {
struct Commands;
class CommandParser {
public:
  std::vector<Command> parse_commands(std::vector<Token>);

private:
};
} // namespace TradingEngine::CLI