#include "CLI/Lexer.hpp"

#include <array>
#include <cctype>
#include <charconv>
#include <stdexcept>

namespace TradingEngine::CLI {

const std::array<Token, 6> supported_literals{
    Token{.type{TokenType::OPEN_PARENTHES}, .value{std::string_view("(")}},
    Token{.type{TokenType::CLOSE_PARENTHES}, .value{std::string_view(")")}},
    Token{.type{TokenType::EQUAL}, .value{std::string_view("=")}},
    Token{.type{TokenType::COMMA}, .value{std::string_view(",")}},
    Token{.type{TokenType::PLUS}, .value{std::string_view("+")}},
    Token{.type{TokenType::MINUS}, .value{std::string_view("-")}},
};

std::vector<Token> Lexer::lex_input(std::string_view input) const {
  std::vector<Token> tokens{};
  while (true) {
    Token next{get_next_token(input)};
    tokens.emplace_back(next);
    if (next.type == TokenType::END_OF_LINE) {
      break;
    }
  }
  return tokens;
}

Token Lexer::get_next_token(std::string_view &input) const {
  if (input.empty()) {
    return Token{.type{TokenType::END_OF_LINE}};
  }

  std::string_view next{scan_next_lexeme(input)};
  if (next.empty()) {
    return Token{.type{TokenType::END_OF_LINE}};
  }

  if (is_string(next)) {
    return Token{.type{TokenType::STRING}, .value{next.substr(1, next.length() - 2)}};
  }

  if (is_number(next)) {
    return Token{.type{TokenType::NUMBER}, .value{next}};
  }

  if (const auto literal = check_literal(next)) {
    return *literal;
  }
  return Token{.type{TokenType::IDENTIFIER}, .value{next}};
}

std::string_view Lexer::scan_next_lexeme(std::string_view &input) const {
  trim_left_white_space(input);
  if (input.empty()) {
    return std::string_view("");
  }
  std::size_t i{0};
  const std::size_t end{input.length()};
  bool is_string{input[0] == '"'}; // can add more conditions
  while (i < end) {
    if (is_string) {
      if (input[i] == '"') {
        if (i == 0) {
          is_string = true;
        } else {
          is_string = false;
          ++i;   // include the closing quote
          break; // the string is complete
        }
      }
    } else if (std::isspace(static_cast<unsigned char>(input[i]))) {
      break;
    } else if (is_literal(input[i])) {
      if (i == 0) {
        i++;
      }
      break;
    }
    i++;
  }
  std::string_view temp{input.substr(0, i)};
  input.remove_prefix(i);
  return temp;
}

void Lexer::trim_left_white_space(std::string_view &input) const {
  std::size_t i{};
  for (; i < input.length(); i++) {
    if (!std::isspace(static_cast<unsigned char>(input[i]))) {
      break;
    }
  }
  input.remove_prefix(i);
}

bool Lexer::is_number(const std::string_view lexeme) const {
  double value{};
  const auto [ptr, ec] = std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), value);

  return ec == std::errc{} && ptr == lexeme.data() + lexeme.size();
}

bool Lexer::is_string(const std::string_view input) const {
  return input.size() >= 2 && input.front() == '"' && input.back() == '"';
};

bool Lexer::is_literal(const std::string_view input) const {
  for (const auto &literal : supported_literals) {
    if (literal.value == input) {
      return true;
    }
  }
  return false;
}

bool Lexer::is_literal(const char input) const {
  for (const auto &literal : supported_literals) {
    if (literal.value[0] == input) {
      return true;
    }
  }
  return false;
}

std::optional<Token> Lexer::check_literal(const std::string_view input) const {
  for (const auto &literal : supported_literals) {
    if (literal.value == input) {
      return literal;
    }
  }
  return std::nullopt;
}

} // namespace TradingEngine::CLI