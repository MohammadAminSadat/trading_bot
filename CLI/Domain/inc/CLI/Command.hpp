#pragma once
#include <string_view>

namespace TradingEngine::CLI {
enum class CommandType {
  UTIL,
  DATA,
  INDICATOR,
  STRATEGY,
  RISK,
  SUMMARY,
  LOGS,
  OTHERS,
};

struct Command {
  CommandType type;
  std::string_view args;
};
} // namespace TradingEngine::CLI