#pragma once
#include <string_view>

namespace TradingEngine::CLI {

enum class TokenType {
  IDENTIFIER,
  SPECIAL,
  NUMBER,
  STRING,
  END,
};

struct Token {
  TokenType type;
  std::string_view value;
};

} // namespace TradingEngine::CLI