#pragma once
#include <string_view>

namespace TradingEngine::CLI {

enum class TokenType {
  IDENTIFIER,
  NUMBER,
  STRING,
  OPEN_PARENTHES,
  CLOSE_PARENTHES,
  EQUAL,
  COMMA,
  PLUS,
  MINUS,
  INVALID,
  END_OF_LINE,
};

struct Token {
  TokenType type;
  std::string_view value;
};

} // namespace TradingEngine::CLI