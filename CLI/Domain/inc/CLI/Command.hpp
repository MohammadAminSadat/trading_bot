#pragma once
#include <string_view>
#include <vector>

namespace TradingEngine::CLI {
enum class CommandType {
  UTILITY_COMMAND,
  DATA_COMMAND,
  INDICATOR_COMMAND,
  STRATEGY_COMMAND,
  RISK_COMMAND,
  BACKTEST_COMMAND,
};

struct Command {
  CommandType type;
  std::string_view sub_command;
  std::vector<std::string_view> arguments;
};
} // namespace TradingEngine::CLI