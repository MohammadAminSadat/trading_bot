#include "CLI/Lexer.hpp"

#include <cctype>
#include <charconv>

namespace TradingEngine::CLI {
std::vector<Token> Lexer::lex_input(std::string_view input) const {
  return lex_string(scan_lexemes(input));
}
std::vector<std::string_view> Lexer::scan_lexemes(std::string_view input) const {
  std::vector<std::string_view> lexemes;
  size_t i{};
  size_t end{input.size()};

  while (i < end) {
    // Skip whitespace.
    while (i < end && std::isspace(static_cast<unsigned char>(input[i]))) {
      ++i;
    }

    auto start{i};

    // Find the end of the lexemes.
    while (i < end && !std::isspace(static_cast<unsigned char>(input[i]))) {
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

  return lexemes;
}

std::vector<Token> Lexer::lex_string(const std::vector<std::string_view> &lexemes) const {
  std::vector<Token> tokens;

  for (const auto &lexeme : lexemes) {
    if (std::ispunct(static_cast<unsigned char>(lexeme[0]))) {
      tokens.emplace_back(Token{
          .type{TokenType::SPECIAL},
          .value{lexeme},
      });
    } else if (is_int(lexeme)) {
      tokens.emplace_back(Token{
          .type{TokenType::NUMBER},
          .value{lexeme},
      });
    } else {
      tokens.emplace_back(Token{
          .type{TokenType::IDENTIFIER},
          .value{lexeme},
      });
    }
  }
  return tokens;
}

bool Lexer::is_int(std::string_view lexeme) const {
  int value{};
  const auto [ptr, ec] = std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), value);

  return ec == std::errc{} && ptr == lexeme.data() + lexeme.size();
}
} // namespace TradingEngine::CLI