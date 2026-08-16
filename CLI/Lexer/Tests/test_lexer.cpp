#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#define private public
#include "CLI/Lexer.hpp"
#undef private

using namespace TradingEngine::CLI;

namespace {

std::vector<Token> lex(const char* input) {
  return Lexer{}.lex_input(input);
}

void expect_token(const Token& token, TokenType type, std::string_view value) {
  EXPECT_EQ(token.type, type);
  EXPECT_EQ(token.value, value);
}

} // namespace

// =============================================================================
// lex_input - end-of-line handling
// =============================================================================

TEST(LexInputTest, EmptyInputReturnsSingleEol) {
  const auto tokens = lex("");
  ASSERT_EQ(tokens.size(), 1);
  expect_token(tokens[0], TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, WhitespaceOnlyReturnsSingleEol) {
  const auto tokens = lex(" \t\n\r\v\f");
  ASSERT_EQ(tokens.size(), 1);
  expect_token(tokens[0], TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, EveryResultEndsWithEol) {
  const std::vector<const char*> inputs{"a", "123", "\"s\"", "(", "a b c 1 2 3"};
  for (const char* input : inputs) {
    const auto tokens = lex(input);
    ASSERT_FALSE(tokens.empty());
    EXPECT_EQ(tokens.back().type, TokenType::END_OF_LINE);
  }
}

TEST(LexInputTest, EolAppearsExactlyOnceAtEnd) {
  const auto tokens = lex("foo(1, 2)");
  std::size_t eol_count{0};
  for (const auto& token : tokens) {
    if (token.type == TokenType::END_OF_LINE) {
      ++eol_count;
    }
  }
  EXPECT_EQ(eol_count, 1);
  EXPECT_EQ(tokens.back().type, TokenType::END_OF_LINE);
}

// =============================================================================
// lex_input - identifiers
// =============================================================================

TEST(LexInputTest, SingleIdentifier) {
  const auto tokens = lex("buy");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::IDENTIFIER, "buy");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, IdentifierWithUnderscoreAndDigits) {
  const auto tokens = lex("sma_200");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::IDENTIFIER, "sma_200");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, MultipleIdentifiers) {
  const auto tokens = lex("alpha beta gamma");
  ASSERT_EQ(tokens.size(), 4);
  expect_token(tokens[0], TokenType::IDENTIFIER, "alpha");
  expect_token(tokens[1], TokenType::IDENTIFIER, "beta");
  expect_token(tokens[2], TokenType::IDENTIFIER, "gamma");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

// =============================================================================
// lex_input - numbers
// =============================================================================

TEST(LexInputTest, IntegerNumber) {
  const auto tokens = lex("42");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, "42");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, DecimalNumber) {
  const auto tokens = lex("3.14");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, "3.14");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, ExponentNumber) {
  const auto tokens = lex("1e5");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, "1e5");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, DecimalExponentNumber) {
  const auto tokens = lex("1.5e3");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, "1.5e3");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, NegativeExponent) {
  const auto tokens = lex("1.5e-3");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, "1.5e-3");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, LeadingDotNumber) {
  const auto tokens = lex(".5");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, ".5");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, TrailingDotNumber) {
  const auto tokens = lex("3.");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, "3.");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, LeadingZeroNumber) {
  const auto tokens = lex("007");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, "007");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, InfinityParsedAsNumber) {
  const auto tokens = lex("inf");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, "inf");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, NanParsedAsNumber) {
  const auto tokens = lex("nan");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::NUMBER, "nan");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, NegativeNumberSplitIntoMinusAndNumber) {
  const auto tokens = lex("-5");
  ASSERT_EQ(tokens.size(), 3);
  expect_token(tokens[0], TokenType::MINUS, "-");
  expect_token(tokens[1], TokenType::NUMBER, "5");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, PlusSignIsLiteralNotNumber) {
  const auto tokens = lex("+3");
  ASSERT_EQ(tokens.size(), 3);
  expect_token(tokens[0], TokenType::PLUS, "+");
  expect_token(tokens[1], TokenType::NUMBER, "3");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

// =============================================================================
// lex_input - strings
// =============================================================================

TEST(LexInputTest, SimpleString) {
  const auto tokens = lex("\"hello\"");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::STRING, "hello");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, EmptyString) {
  const auto tokens = lex("\"\"");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::STRING, "");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, StringWithSpaces) {
  const auto tokens = lex("\"hello world\"");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::STRING, "hello world");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, StringSurroundedByWhitespace) {
  const auto tokens = lex("  \"hello\"  ");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::STRING, "hello");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, StringIsTrimmedOfQuotes) {
  const auto tokens = lex("\"   spaced   \"");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::STRING, "   spaced   ");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

// =============================================================================
// lex_input - literals
// =============================================================================

TEST(LexInputTest, EachLiteralProducesCorrectToken) {
  struct Case {
    const char* input;
    TokenType type;
    const char* value;
  };
  const std::vector<Case> cases{
      {"(", TokenType::OPEN_PARENTHES, "("}, {")", TokenType::CLOSE_PARENTHES, ")"},
      {"=", TokenType::EQUAL, "="},          {",", TokenType::COMMA, ","},
      {"+", TokenType::PLUS, "+"},           {"-", TokenType::MINUS, "-"},
  };
  for (const auto& c : cases) {
    const auto tokens = lex(c.input);
    ASSERT_EQ(tokens.size(), 2) << "input: " << c.input;
    expect_token(tokens[0], c.type, c.value);
    expect_token(tokens.back(), TokenType::END_OF_LINE, "");
  }
}

TEST(LexInputTest, AdjacentLiterals) {
  const auto tokens = lex("()=,+");
  ASSERT_EQ(tokens.size(), 6);
  expect_token(tokens[0], TokenType::OPEN_PARENTHES, "(");
  expect_token(tokens[1], TokenType::CLOSE_PARENTHES, ")");
  expect_token(tokens[2], TokenType::EQUAL, "=");
  expect_token(tokens[3], TokenType::COMMA, ",");
  expect_token(tokens[4], TokenType::PLUS, "+");
  expect_token(tokens[5], TokenType::END_OF_LINE, "");
}

// =============================================================================
// lex_input - full commands and mixed input
// =============================================================================

TEST(LexInputTest, FullCommand) {
  const auto tokens = lex("strategy(rsi=14, close)");
  ASSERT_EQ(tokens.size(), 9);
  expect_token(tokens[0], TokenType::IDENTIFIER, "strategy");
  expect_token(tokens[1], TokenType::OPEN_PARENTHES, "(");
  expect_token(tokens[2], TokenType::IDENTIFIER, "rsi");
  expect_token(tokens[3], TokenType::EQUAL, "=");
  expect_token(tokens[4], TokenType::NUMBER, "14");
  expect_token(tokens[5], TokenType::COMMA, ",");
  expect_token(tokens[6], TokenType::IDENTIFIER, "close");
  expect_token(tokens[7], TokenType::CLOSE_PARENTHES, ")");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, CommandWithStringValue) {
  const auto tokens = lex("set(name=\"btc\", pair=\"btc/usdt\")");
  ASSERT_EQ(tokens.size(), 11);
  expect_token(tokens[0], TokenType::IDENTIFIER, "set");
  expect_token(tokens[1], TokenType::OPEN_PARENTHES, "(");
  expect_token(tokens[2], TokenType::IDENTIFIER, "name");
  expect_token(tokens[3], TokenType::EQUAL, "=");
  expect_token(tokens[4], TokenType::STRING, "btc");
  expect_token(tokens[5], TokenType::COMMA, ",");
  expect_token(tokens[6], TokenType::IDENTIFIER, "pair");
  expect_token(tokens[7], TokenType::EQUAL, "=");
  expect_token(tokens[8], TokenType::STRING, "btc/usdt");
  expect_token(tokens[9], TokenType::CLOSE_PARENTHES, ")");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, MixedWhitespaceBetweenTokens) {
  const auto tokens = lex("  sma_200\t( )  ");
  ASSERT_EQ(tokens.size(), 4);
  expect_token(tokens[0], TokenType::IDENTIFIER, "sma_200");
  expect_token(tokens[1], TokenType::OPEN_PARENTHES, "(");
  expect_token(tokens[2], TokenType::CLOSE_PARENTHES, ")");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

// =============================================================================
// lex_input - negative / malformed input
// =============================================================================

TEST(LexInputTest, NumberWithTrailingLettersBecomesIdentifier) {
  const auto tokens = lex("123abc");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::IDENTIFIER, "123abc");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, MultipleDecimalPointsBecomesIdentifier) {
  const auto tokens = lex("1.2.3");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::IDENTIFIER, "1.2.3");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, UnderscoreSeparatedNumberBecomesIdentifier) {
  const auto tokens = lex("1_000");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::IDENTIFIER, "1_000");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, UnterminatedStringBecomesIdentifier) {
  const auto tokens = lex("\"abc");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::IDENTIFIER, "\"abc");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, LeadingQuoteAloneBecomesIdentifier) {
  const auto tokens = lex("\"");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::IDENTIFIER, "\"");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, TrailingQuoteBecomesIdentifier) {
  const auto tokens = lex("abc\"");
  ASSERT_EQ(tokens.size(), 2);
  expect_token(tokens[0], TokenType::IDENTIFIER, "abc\"");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

TEST(LexInputTest, StringFollowedByTokensSplitsAtClosingQuote) {
  const auto tokens = lex("\"a\"b");
  ASSERT_EQ(tokens.size(), 3);
  expect_token(tokens[0], TokenType::STRING, "a");
  expect_token(tokens[1], TokenType::IDENTIFIER, "b");
  expect_token(tokens.back(), TokenType::END_OF_LINE, "");
}

// =============================================================================
// get_next_token (tested directly through the public interface)
// =============================================================================

TEST(GetNextTokenTest, EmptyInputReturnsEol) {
  Lexer lexer;
  std::string_view input{};
  const auto token = lexer.get_next_token(input);
  expect_token(token, TokenType::END_OF_LINE, "");
}

TEST(GetNextTokenTest, WhitespaceOnlyReturnsEol) {
  Lexer lexer;
  std::string_view input{"   "};
  const auto token = lexer.get_next_token(input);
  expect_token(token, TokenType::END_OF_LINE, "");
  EXPECT_TRUE(input.empty());
}

TEST(GetNextTokenTest, ReturnsIdentifierAndAdvances) {
  Lexer lexer;
  std::string_view input{"foo bar"};
  const auto token = lexer.get_next_token(input);
  expect_token(token, TokenType::IDENTIFIER, "foo");
  EXPECT_EQ(input, std::string_view{" bar"});
}

TEST(GetNextTokenTest, ReturnsNumberAndAdvances) {
  Lexer lexer;
  std::string_view input{"10 rest"};
  const auto token = lexer.get_next_token(input);
  expect_token(token, TokenType::NUMBER, "10");
  EXPECT_EQ(input, std::string_view{" rest"});
}

TEST(GetNextTokenTest, ReturnsStringAndAdvances) {
  Lexer lexer;
  std::string_view input{"\"x y\" tail"};
  const auto token = lexer.get_next_token(input);
  expect_token(token, TokenType::STRING, "x y");
  EXPECT_EQ(input, std::string_view{" tail"});
}

TEST(GetNextTokenTest, ReturnsLiteralAndAdvances) {
  Lexer lexer;
  std::string_view input{"=5"};
  const auto token = lexer.get_next_token(input);
  expect_token(token, TokenType::EQUAL, "=");
  EXPECT_EQ(input, std::string_view{"5"});
}

// =============================================================================
// scan_next_lexeme
// =============================================================================

TEST(ScanNextLexemeTest, EmptyInputReturnsEmpty) {
  Lexer lexer;
  std::string_view input{};
  EXPECT_TRUE(lexer.scan_next_lexeme(input).empty());
}

TEST(ScanNextLexemeTest, WhitespaceOnlyReturnsEmptyAndConsumes) {
  Lexer lexer;
  std::string_view input{"   "};
  EXPECT_TRUE(lexer.scan_next_lexeme(input).empty());
  EXPECT_TRUE(input.empty());
}

TEST(ScanNextLexemeTest, ReturnsIdentifierLexeme) {
  Lexer lexer;
  std::string_view input{"hello world"};
  EXPECT_EQ(lexer.scan_next_lexeme(input), std::string_view{"hello"});
  EXPECT_EQ(input, std::string_view{" world"});
}

TEST(ScanNextLexemeTest, ReturnsNumberLexeme) {
  Lexer lexer;
  std::string_view input{"3.14 abc"};
  EXPECT_EQ(lexer.scan_next_lexeme(input), std::string_view{"3.14"});
  EXPECT_EQ(input, std::string_view{" abc"});
}

TEST(ScanNextLexemeTest, ReturnsLiteralLexeme) {
  Lexer lexer;
  std::string_view input{"(abc"};
  EXPECT_EQ(lexer.scan_next_lexeme(input), std::string_view{"("});
  EXPECT_EQ(input, std::string_view{"abc"});
}

TEST(ScanNextLexemeTest, ReturnsStringLexemeIncludingSpaces) {
  Lexer lexer;
  std::string_view input{"\"hello world\" rest"};
  EXPECT_EQ(lexer.scan_next_lexeme(input), std::string_view{"\"hello world\""});
  EXPECT_EQ(input, std::string_view{" rest"});
}

TEST(ScanNextLexemeTest, TrimsLeadingWhitespaceBeforeLexeme) {
  Lexer lexer;
  std::string_view input{"   token"};
  EXPECT_EQ(lexer.scan_next_lexeme(input), std::string_view{"token"});
  EXPECT_TRUE(input.empty());
}

TEST(ScanNextLexemeTest, UnterminatedStringScansToEnd) {
  Lexer lexer;
  std::string_view input{"\"unterminated"};
  EXPECT_EQ(lexer.scan_next_lexeme(input), std::string_view{"\"unterminated"});
  EXPECT_TRUE(input.empty());
}

// =============================================================================
// trim_left_white_space
// =============================================================================

TEST(TrimLeftWhiteSpaceTest, TrimsSpacesTabsAndNewlines) {
  Lexer lexer;
  std::string_view input{" \t\n\r\v\fabc"};
  lexer.trim_left_white_space(input);
  EXPECT_EQ(input, std::string_view{"abc"});
}

TEST(TrimLeftWhiteSpaceTest, EmptyStaysEmpty) {
  Lexer lexer;
  std::string_view input{};
  lexer.trim_left_white_space(input);
  EXPECT_TRUE(input.empty());
}

TEST(TrimLeftWhiteSpaceTest, NoLeadingWhitespaceIsUnchanged) {
  Lexer lexer;
  std::string_view input{"abc  "};
  lexer.trim_left_white_space(input);
  EXPECT_EQ(input, std::string_view{"abc  "});
}

TEST(TrimLeftWhiteSpaceTest, AllWhitespaceBecomesEmpty) {
  Lexer lexer;
  std::string_view input{"   "};
  lexer.trim_left_white_space(input);
  EXPECT_TRUE(input.empty());
}

// =============================================================================
// is_number
// =============================================================================

TEST(IsNumberTest, ValidNumbersReturnTrue) {
  Lexer lexer;
  EXPECT_TRUE(lexer.is_number("0"));
  EXPECT_TRUE(lexer.is_number("42"));
  EXPECT_TRUE(lexer.is_number("-3"));
  EXPECT_TRUE(lexer.is_number("3.14"));
  EXPECT_TRUE(lexer.is_number(".5"));
  EXPECT_TRUE(lexer.is_number("3."));
  EXPECT_TRUE(lexer.is_number("1e5"));
  EXPECT_TRUE(lexer.is_number("1.5e-3"));
  EXPECT_TRUE(lexer.is_number("inf"));
  EXPECT_TRUE(lexer.is_number("nan"));
}

TEST(IsNumberTest, InvalidNumbersReturnFalse) {
  Lexer lexer;
  EXPECT_FALSE(lexer.is_number(""));
  EXPECT_FALSE(lexer.is_number("abc"));
  EXPECT_FALSE(lexer.is_number("12abc"));
  EXPECT_FALSE(lexer.is_number("1.2.3"));
  EXPECT_FALSE(lexer.is_number("+3"));
  EXPECT_FALSE(lexer.is_number("1_000"));
  EXPECT_FALSE(lexer.is_number(" 12"));
  EXPECT_FALSE(lexer.is_number("12 "));
}

// =============================================================================
// is_string
// =============================================================================

TEST(IsStringTest, QuotedContentReturnsTrue) {
  Lexer lexer;
  EXPECT_TRUE(lexer.is_string("\"abc\""));
  EXPECT_TRUE(lexer.is_string("\"\""));
  EXPECT_TRUE(lexer.is_string("\"hello world\""));
}

TEST(IsStringTest, NonStringsReturnFalse) {
  Lexer lexer;
  EXPECT_FALSE(lexer.is_string(""));
  EXPECT_FALSE(lexer.is_string("\""));
  EXPECT_FALSE(lexer.is_string("abc"));
  EXPECT_FALSE(lexer.is_string("\"abc"));
  EXPECT_FALSE(lexer.is_string("abc\""));
  EXPECT_FALSE(lexer.is_string("a\"b"));
}

// =============================================================================
// is_literal (string_view overload)
// =============================================================================

TEST(IsLiteralStringViewTest, KnownLiteralsReturnTrue) {
  Lexer lexer;
  EXPECT_TRUE(lexer.is_literal(std::string_view{"("}));
  EXPECT_TRUE(lexer.is_literal(std::string_view{")"}));
  EXPECT_TRUE(lexer.is_literal(std::string_view{"="}));
  EXPECT_TRUE(lexer.is_literal(std::string_view{","}));
  EXPECT_TRUE(lexer.is_literal(std::string_view{"+"}));
  EXPECT_TRUE(lexer.is_literal(std::string_view{"-"}));
}

TEST(IsLiteralStringViewTest, UnknownValuesReturnFalse) {
  Lexer lexer;
  EXPECT_FALSE(lexer.is_literal(std::string_view{""}));
  EXPECT_FALSE(lexer.is_literal(std::string_view{"++"}));
  EXPECT_FALSE(lexer.is_literal(std::string_view{"foo"}));
  EXPECT_FALSE(lexer.is_literal(std::string_view{"(("}));
}

TEST(IsLiteralStringViewTest, MultiCharacterNeverMatches) {
  Lexer lexer;
  EXPECT_FALSE(lexer.is_literal(std::string_view{"()"}));
  EXPECT_FALSE(lexer.is_literal(std::string_view{"=="}));
  EXPECT_FALSE(lexer.is_literal(std::string_view{"  ("}));
}

// =============================================================================
// is_literal (char overload)
// =============================================================================

TEST(IsLiteralCharTest, KnownLiteralCharsReturnTrue) {
  Lexer lexer;
  EXPECT_TRUE(lexer.is_literal('('));
  EXPECT_TRUE(lexer.is_literal(')'));
  EXPECT_TRUE(lexer.is_literal('='));
  EXPECT_TRUE(lexer.is_literal(','));
  EXPECT_TRUE(lexer.is_literal('+'));
  EXPECT_TRUE(lexer.is_literal('-'));
}

TEST(IsLiteralCharTest, UnknownCharsReturnFalse) {
  Lexer lexer;
  EXPECT_FALSE(lexer.is_literal('a'));
  EXPECT_FALSE(lexer.is_literal('1'));
  EXPECT_FALSE(lexer.is_literal(' '));
  EXPECT_FALSE(lexer.is_literal('"'));
  EXPECT_FALSE(lexer.is_literal('\0'));
}

// =============================================================================
// check_literal
// =============================================================================

TEST(CheckLiteralTest, KnownLiteralReturnsToken) {
  Lexer lexer;
  auto token = lexer.check_literal(std::string_view{"("});
  ASSERT_TRUE(token.has_value());
  expect_token(*token, TokenType::OPEN_PARENTHES, "(");
}

TEST(CheckLiteralTest, AllSupportedLiteralsAreResolved) {
  Lexer lexer;
  struct Case {
    std::string_view value;
    TokenType type;
  };
  const std::vector<Case> cases{
      {"(", TokenType::OPEN_PARENTHES}, {")", TokenType::CLOSE_PARENTHES},
      {"=", TokenType::EQUAL},          {",", TokenType::COMMA},
      {"+", TokenType::PLUS},           {"-", TokenType::MINUS},
  };
  for (const auto& c : cases) {
    auto token = lexer.check_literal(c.value);
    ASSERT_TRUE(token.has_value()) << "value: " << c.value;
    EXPECT_EQ(token->type, c.type);
    EXPECT_EQ(token->value, c.value);
  }
}

TEST(CheckLiteralTest, UnknownValueReturnsNullopt) {
  Lexer lexer;
  EXPECT_FALSE(lexer.check_literal(std::string_view{""}).has_value());
  EXPECT_FALSE(lexer.check_literal(std::string_view{"foo"}).has_value());
  EXPECT_FALSE(lexer.check_literal(std::string_view{"(("}).has_value());
}

// =============================================================================
// Token structure sanity
// =============================================================================

TEST(TokenTest, DefaultConstructedTokenHasEmptyValue) {
  Token token{};
  EXPECT_TRUE(token.value.empty());
}

TEST(TokenTest, AggregateInitialization) {
  Token token{.type{TokenType::NUMBER}, .value{std::string_view("1")}};
  EXPECT_EQ(token.type, TokenType::NUMBER);
  EXPECT_EQ(token.value, std::string_view{"1"});
}
