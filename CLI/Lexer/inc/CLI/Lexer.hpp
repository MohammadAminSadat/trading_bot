#pragma once
#include <CLI/Token.hpp>
#include <string_view>
#include <vector>

namespace TradingEngine::CLI {

class Lexer {
public:
  std::vector<Token> lex_input(std::string_view) const;

private:
  std::vector<std::string_view> scan_lexemes(std::string_view) const;
  std::vector<Token> lex_string(const std::vector<std::string_view> &) const;
  bool is_int(std::string_view) const;
};
} // namespace TradingEngine::CLI