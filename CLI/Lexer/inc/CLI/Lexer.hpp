#pragma once
#include <CLI/Token.hpp>
#include <string_view>
#include <vector>

namespace TradingEngine::CLI {
class Lexer {
public:
  std::vector<Token> lex_input(std::string_view) const;

private:
  std::vector<std::string_view> scan_lexemes(const std::string_view) const;
  const std::vector<std::string_view> rule_check(const std::vector<std::string_view> &) const;
  std::vector<Token> lex_string(const std::vector<std::string_view> &) const;
  bool is_number(const std::string_view) const;
  bool is_int(const char c) const;
};
} // namespace TradingEngine::CLI