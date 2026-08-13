#pragma once
#include <string_view>

namespace TradingEngine::CLI {

enum class TokenType {
  IDENTIFIER,
  SPECIAL,
  NUMBER,
  STRING,
  END_OF_LINE,
};

struct Token {
  TokenType type;
  std::string_view value;
};

} // namespace TradingEngine::CLI