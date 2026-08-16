#pragma once
#include <CLI/Token.hpp>
#include <optional>
#include <string_view>
#include <vector>

namespace TradingEngine::CLI {
class Lexer {
public:
  std::vector<Token> lex_input(std::string_view) const;

private:
  bool is_number(const std::string_view) const;
  bool is_string(const std::string_view) const;
  bool is_literal(const std::string_view) const;
  bool is_literal(const char) const;
  std::optional<Token> check_literal(const std::string_view) const;
  void trim_left_white_space(std::string_view &) const;

  Token get_next_token(std::string_view &) const;
  std::string_view scan_next_lexeme(std::string_view &) const;
};
} // namespace TradingEngine::CLI