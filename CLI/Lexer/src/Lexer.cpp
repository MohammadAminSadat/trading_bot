#include "CLI/Lexer.hpp"

#include <cctype>
#include <charconv>
#include <stdexcept>

namespace TradingEngine::CLI {
std::vector<Token> Lexer::lex_input(std::string_view input) const {
  return lex_string(scan_lexemes(input));
}
std::vector<std::string_view> Lexer::scan_lexemes(const std::string_view input) const {
  std::vector<std::string_view> lexemes;
  std::size_t i{};
  std::size_t end{input.size()};
  bool quote_start{false};

  while (i < end) {
    while (i < end && std::isspace(static_cast<unsigned char>(input[i]))) {
      ++i;
    }

    auto start{i};

    while ((i < end && !std::isspace(static_cast<unsigned char>(input[i]))) || quote_start) {
      if (input[i] == '.') {
        if (i + 1 < end && is_int(input[i + 1]) || (i > 0 && is_int(input[i - 1]))) {
          i++;
          continue;
        }
      }
      if (input[i] == '"') {
        if (!quote_start && i > 0) {
          if (!std::isspace(static_cast<unsigned char>(input[i - 1]))) {
            throw std::invalid_argument("String must start with a white space and \".");
          }
        }
        if (quote_start && i + 1 < end) {
          if (!std::isspace(static_cast<unsigned char>(input[i + 1]))) {
            throw std::invalid_argument("String must stop with \" and white space.");
          }
        }
        quote_start = !quote_start;
        i++;
        continue;
      }
      if (std::ispunct(static_cast<unsigned char>(input[i]))) {
        if (start != i) {
          lexemes.emplace_back(input.substr(start, i - start));
        }

        lexemes.emplace_back(input.substr(i, 1));

        start = i + 1;
      }
      ++i;
    }

    if (start != i) {
      lexemes.emplace_back(input.substr(start, i - start));
    }
  }

  if (quote_start) {
    throw std::invalid_argument("Opened Double quote is not closed!");
  }

  return lexemes;
}

const std::vector<std::string_view> Lexer::rule_check(
    const std::vector<std::string_view> &lexemes) const {
  std::vector<std::string_view> checked_lexemes{};
  std::string_view k;
  std::size_t i{};
  const std::size_t size{lexemes.size()};
  bool quote_open{false};
  while (i < size) {
    if (lexemes[i] == ".") {
      if (i > 1 && i + 1 < size) {
      }
    }
  }
  return checked_lexemes;
}

std::vector<Token> Lexer::lex_string(const std::vector<std::string_view> &lexemes) const {
  std::vector<Token> tokens;

  for (const auto &lexeme : lexemes) {
    if (is_number(lexeme)) {
      tokens.emplace_back(Token{
          .type{TokenType::NUMBER},
          .value{lexeme},
      });
    } else if (lexeme[0] == '"') {
      tokens.emplace_back(Token{
          .type{TokenType::STRING},
          .value{lexeme},
      });
    } else if (std::ispunct(static_cast<unsigned char>(lexeme[0]))) {
      tokens.emplace_back(Token{
          .type{TokenType::SPECIAL},
          .value{lexeme},
      });
    }

    else {
      tokens.emplace_back(Token{
          .type{TokenType::IDENTIFIER},
          .value{lexeme},
      });
    }
  }
  tokens.emplace_back(Token{
      .type{TokenType::END_OF_LINE},
      .value{""},
  });
  return tokens;
}

bool Lexer::is_number(const std::string_view lexeme) const {
  double value{};
  const auto [ptr, ec] = std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), value);

  return ec == std::errc{} && ptr == lexeme.data() + lexeme.size();
}

bool Lexer::is_int(const char c) const {
  return std::isdigit(static_cast<unsigned char>(c)) != 0;
}

} // namespace TradingEngine::CLI