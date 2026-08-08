#include <MarketData/CSVProvider.hpp>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <utility>

namespace TEC = TradingEngine::Core;
namespace TEM = TradingEngine::MarketData;

namespace {

using Candle = TEM::Candle;
using CSVProvider = TEM::CSVProvider;
using CSVImporterConfiguration = TEM::CSVImporterConfiguration;

constexpr const char *CSV_HEADER = "timestamp,open,high,low,close,volume\n";
constexpr const char *CSV_VALID_ROW_1 = "2010-07-24 20:00,1.0,1.1,0.9,1.05,100\n";
constexpr const char *CSV_VALID_ROW_2 = "2010-07-24 20:01,1.2,1.3,1.1,1.25,200\n";

struct TempFile {
  std::filesystem::path path;

  explicit TempFile(const std::string &content) {
    static std::atomic<unsigned> counter{0};
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    auto name = "csv_provider_test_" + std::to_string(now) + "_" + std::to_string(::getpid()) +
                "_" + std::to_string(++counter) + ".csv";
    path = std::filesystem::temp_directory_path() / name;
    std::ofstream ofs(path);
    ofs << content;
  }

  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path, ec);
  }
};

CSVImporterConfiguration make_config(const TempFile &file) {
  CSVImporterConfiguration config;
  config.path = file.path;
  return config;
}

} // namespace

// =============================================================================
// CSVProvider construction tests
// =============================================================================

TEST(CSVProviderConstruction, SucceedsWithValidFile) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1 + CSV_VALID_ROW_2};
  EXPECT_NO_THROW(CSVProvider{make_config(file)});
}

TEST(CSVProviderConstruction, ThrowsWhenFileDoesNotExist) {
  CSVImporterConfiguration config;
  config.path = std::filesystem::temp_directory_path() / "no_such_csv_provider_file.csv";
  EXPECT_THROW(CSVProvider{std::move(config)}, std::runtime_error);
}

TEST(CSVProviderConstruction, ThrowsWhenFileHasOnlyHeader) {
  TempFile file{std::string{CSV_HEADER}};
  EXPECT_THROW(CSVProvider{make_config(file)}, std::runtime_error);
}

TEST(CSVProviderConstruction, ThrowsWhenFileIsEmpty) {
  TempFile file{""};
  EXPECT_THROW(CSVProvider{make_config(file)}, std::runtime_error);
}

TEST(CSVProviderConstruction, ThrowsWhenColumnMappingIsOutOfBounds) {
  TempFile file{"timestamp,close\n2010-07-24 20:00,1.05\n"};
  CSVImporterConfiguration config = make_config(file);
  config.column_mapping = {.timestamp{0}, .open{2}, .high{2}, .low{2}, .close{1}, .volume{}};
  EXPECT_THROW(CSVProvider{std::move(config)}, std::runtime_error);
}

TEST(CSVProviderConstruction, ThrowsWhenOptionalVolumeColumnIsOutOfBounds) {
  TempFile file{"timestamp,open,high,low,close\n2010-07-24 20:00,1.0,1.1,0.9,1.05\n"};
  CSVImporterConfiguration config = make_config(file);
  config.column_mapping = {.timestamp{0}, .open{1}, .high{2}, .low{3}, .close{4}, .volume{5}};
  EXPECT_THROW(CSVProvider{std::move(config)}, std::runtime_error);
}

TEST(CSVProviderConstruction, DefaultMappingMatchesDefaultFile) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1};
  CSVImporterConfiguration config = make_config(file);
  EXPECT_EQ(config.column_mapping.timestamp, 0u);
  EXPECT_EQ(config.column_mapping.open, 1u);
  EXPECT_EQ(config.column_mapping.high, 2u);
  EXPECT_EQ(config.column_mapping.low, 3u);
  EXPECT_EQ(config.column_mapping.close, 4u);
  EXPECT_EQ(config.column_mapping.volume, std::optional<size_t>(5));
  EXPECT_TRUE(config.has_header);
  EXPECT_EQ(config.delimiter, ",");
}

// =============================================================================
// CSVProvider get_next tests
// =============================================================================

TEST(CSVProviderGetNext, ParsesValidRowIntoCandle) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1};
  CSVProvider provider{make_config(file)};

  auto candle = provider.get_next();
  ASSERT_TRUE(candle.has_value());
  EXPECT_DOUBLE_EQ(candle->open, 1.0);
  EXPECT_DOUBLE_EQ(candle->high, 1.1);
  EXPECT_DOUBLE_EQ(candle->low, 0.9);
  EXPECT_DOUBLE_EQ(candle->close, 1.05);
  EXPECT_EQ(candle->volume, std::optional<int>(100));
  EXPECT_EQ(candle->timestamp, TEC::make_timestamp(2010, 7, 24, 20, 0));
}

TEST(CSVProviderGetNext, ReturnsCandlesInFileOrder) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1 + CSV_VALID_ROW_2};
  CSVProvider provider{make_config(file)};

  auto first = provider.get_next();
  auto second = provider.get_next();
  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());
  EXPECT_EQ(first->timestamp, TEC::make_timestamp(2010, 7, 24, 20, 0));
  EXPECT_EQ(second->timestamp, TEC::make_timestamp(2010, 7, 24, 20, 1));
}

TEST(CSVProviderGetNext, ReturnsNulloptWhenExhausted) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1};
  CSVProvider provider{make_config(file)};

  ASSERT_TRUE(provider.get_next().has_value());
  EXPECT_FALSE(provider.get_next().has_value());
}

TEST(CSVProviderGetNext, KeepsReturningNulloptAfterExhaustion) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1};
  CSVProvider provider{make_config(file)};

  provider.get_next();
  EXPECT_FALSE(provider.get_next().has_value());
  EXPECT_FALSE(provider.get_next().has_value());
  EXPECT_FALSE(provider.get_next().has_value());
}

TEST(CSVProviderGetNext, ReadsFileWithoutHeader) {
  TempFile file{std::string{CSV_VALID_ROW_1} + CSV_VALID_ROW_2};
  CSVImporterConfiguration config = make_config(file);
  config.has_header = false;
  CSVProvider provider{std::move(config)};

  ASSERT_TRUE(provider.get_next().has_value());
  ASSERT_TRUE(provider.get_next().has_value());
  EXPECT_FALSE(provider.get_next().has_value());
}

TEST(CSVProviderGetNext, SupportsCustomDelimiter) {
  TempFile file{"timestamp;open;high;low;close;volume\n2010-07-24 20:00;1.0;1.1;0.9;1.05;100\n"};
  CSVImporterConfiguration config = make_config(file);
  config.delimiter = ";";
  CSVProvider provider{std::move(config)};

  auto candle = provider.get_next();
  ASSERT_TRUE(candle.has_value());
  EXPECT_DOUBLE_EQ(candle->close, 1.05);
  EXPECT_EQ(candle->volume, std::optional<int>(100));
}

TEST(CSVProviderGetNext, SupportsCustomColumnMapping) {
  TempFile file{"timestamp,close,high,low,open,volume\n2010-07-24 20:00,1.05,1.1,0.9,1.0,100\n"};
  CSVImporterConfiguration config = make_config(file);
  config.column_mapping = {.timestamp{0}, .open{4}, .high{2}, .low{3}, .close{1}, .volume{5}};
  CSVProvider provider{std::move(config)};

  auto candle = provider.get_next();
  ASSERT_TRUE(candle.has_value());
  EXPECT_DOUBLE_EQ(candle->open, 1.0);
  EXPECT_DOUBLE_EQ(candle->high, 1.1);
  EXPECT_DOUBLE_EQ(candle->low, 0.9);
  EXPECT_DOUBLE_EQ(candle->close, 1.05);
  EXPECT_EQ(candle->volume, std::optional<int>(100));
}

TEST(CSVProviderGetNext, LeavesVolumeEmptyWhenNotMapped) {
  TempFile file{"timestamp,open,high,low,close\n2010-07-24 20:00,1.0,1.1,0.9,1.05\n"};
  CSVImporterConfiguration config = make_config(file);
  config.column_mapping = {.timestamp{0}, .open{1}, .high{2}, .low{3}, .close{4}, .volume{}};
  CSVProvider provider{std::move(config)};

  auto candle = provider.get_next();
  ASSERT_TRUE(candle.has_value());
  EXPECT_FALSE(candle->volume.has_value());
}

TEST(CSVProviderGetNext, UsesCustomTimestampParser) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1};
  CSVImporterConfiguration config = make_config(file);
  const auto expected = TEC::make_timestamp(2020, 1, 2, 3, 4);
  config.timestamp_parser = [expected](std::string_view) { return expected; };
  CSVProvider provider{std::move(config)};

  auto candle = provider.get_next();
  ASSERT_TRUE(candle.has_value());
  EXPECT_EQ(candle->timestamp, expected);
}

TEST(CSVProviderGetNext, ThrowsOnMalformedTimestamp) {
  TempFile file{std::string{CSV_HEADER} + "not-a-timestamp,1.0,1.1,0.9,1.05,100\n"};
  CSVProvider provider{make_config(file)};
  EXPECT_THROW(provider.get_next(), std::runtime_error);
}

TEST(CSVProviderGetNext, ThrowsOnInvalidDate) {
  TempFile file{std::string{CSV_HEADER} + "2010-13-24 20:00,1.0,1.1,0.9,1.05,100\n"};
  CSVProvider provider{make_config(file)};
  EXPECT_THROW(provider.get_next(), std::invalid_argument);
}

TEST(CSVProviderGetNext, ThrowsOnInvalidPrice) {
  TempFile file{std::string{CSV_HEADER} + "2010-07-24 20:00,abc,1.1,0.9,1.05,100\n"};
  CSVProvider provider{make_config(file)};
  EXPECT_THROW(provider.get_next(), std::invalid_argument);
}

TEST(CSVProviderGetNext, ThrowsOnInvalidVolume) {
  TempFile file{std::string{CSV_HEADER} + "2010-07-24 20:00,1.0,1.1,0.9,1.05,not-a-number\n"};
  CSVProvider provider{make_config(file)};
  EXPECT_THROW(provider.get_next(), std::invalid_argument);
}

TEST(CSVProviderGetNext, ThrowsWhenCandleIsInvalid) {
  TempFile file{std::string{CSV_HEADER} + "2010-07-24 20:00,1.0,0.9,1.1,1.05,100\n"};
  CSVProvider provider{make_config(file)};
  EXPECT_THROW(provider.get_next(), std::runtime_error);
}

TEST(CSVProviderGetNext, ThrowsWhenVolumeIsNegative) {
  TempFile file{std::string{CSV_HEADER} + "2010-07-24 20:00,1.0,1.1,0.9,1.05,-5\n"};
  CSVProvider provider{make_config(file)};
  EXPECT_THROW(provider.get_next(), std::runtime_error);
}

TEST(CSVProviderGetNext, ThrowsOnNonFinitePrice) {
  TempFile file{std::string{CSV_HEADER} + "2010-07-24 20:00,inf,1.1,0.9,1.05,100\n"};
  CSVProvider provider{make_config(file)};
  EXPECT_THROW(provider.get_next(), std::runtime_error);
}

// =============================================================================
// CSVProvider reset tests
// =============================================================================

TEST(CSVProviderReset, RestartsFromBeginning) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1 + CSV_VALID_ROW_2};
  CSVProvider provider{make_config(file)};

  ASSERT_TRUE(provider.get_next().has_value());
  ASSERT_TRUE(provider.get_next().has_value());
  EXPECT_FALSE(provider.get_next().has_value());

  provider.reset();
  auto candle = provider.get_next();
  ASSERT_TRUE(candle.has_value());
  EXPECT_EQ(candle->timestamp, TEC::make_timestamp(2010, 7, 24, 20, 0));
}

// =============================================================================
// CSVProvider iterator tests
// =============================================================================

TEST(CSVProviderIterator, RangeForYieldsAllCandles) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1 + CSV_VALID_ROW_2};
  CSVProvider provider{make_config(file)};

  size_t count{0};
  for (const auto &candle : provider) {
    EXPECT_DOUBLE_EQ(candle.close, count == 0 ? 1.05 : 1.25);
    ++count;
  }
  EXPECT_EQ(count, 2u);
}

TEST(CSVProviderIterator, IncrementsToSentinel) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1};
  CSVProvider provider{make_config(file)};

  auto it = provider.begin();
  EXPECT_NE(it, provider.end());
  ++it;
  EXPECT_EQ(it, provider.end());
  EXPECT_EQ(provider.end(), it);
}

TEST(CSVProviderIterator, PostIncrementReturnsPreviousElement) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1 + CSV_VALID_ROW_2};
  CSVProvider provider{make_config(file)};

  auto it = provider.begin();
  auto previous = it++;
  EXPECT_EQ(previous->timestamp, TEC::make_timestamp(2010, 7, 24, 20, 0));
  EXPECT_EQ(it->timestamp, TEC::make_timestamp(2010, 7, 24, 20, 1));
}

TEST(CSVProviderIterator, ArrowOperatorAccessesFields) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1};
  CSVProvider provider{make_config(file)};

  auto it = provider.begin();
  EXPECT_DOUBLE_EQ(it->open, 1.0);
  EXPECT_DOUBLE_EQ(it->high, 1.1);
  EXPECT_DOUBLE_EQ(it->close, 1.05);
}

TEST(CSVProviderIterator, DefaultConstructedIteratorEqualsSentinel) {
  TEM::CSVProvider::Iterator end;
  EXPECT_EQ(end, std::default_sentinel);
  EXPECT_EQ(std::default_sentinel, end);
}

TEST(CSVProviderIterator, IncrementEndThrowsLogicError) {
  TEM::CSVProvider::Iterator end;
  EXPECT_THROW(++end, std::logic_error);
  EXPECT_THROW(end++, std::logic_error);
}

TEST(CSVProviderIterator, DoesNotLeakAcrossRestartedIterations) {
  TempFile file{std::string{CSV_HEADER} + CSV_VALID_ROW_1};
  CSVProvider provider{make_config(file)};

  size_t first_pass{0};
  for (const auto &candle : provider) {
    (void)candle;
    ++first_pass;
  }
  size_t second_pass{0};
  for (const auto &candle : provider) {
    (void)candle;
    ++second_pass;
  }
  EXPECT_EQ(first_pass, 1u);
  EXPECT_EQ(second_pass, 1u);
}

// =============================================================================
// default_timestamp_parser tests
// =============================================================================

TEST(DefaultTimestampParser, ParsesValidFormat) {
  EXPECT_EQ(TEM::default_timestamp_parser("2010-07-20 20:00"),
            TEC::make_timestamp(2010, 7, 20, 20, 0));
}

TEST(DefaultTimestampParser, ParsesMinutePrecision) {
  EXPECT_EQ(TEM::default_timestamp_parser("2010-07-20 20:59"),
            TEC::make_timestamp(2010, 7, 20, 20, 59));
}

TEST(DefaultTimestampParser, ThrowsOnWrongLength) {
  EXPECT_THROW(TEM::default_timestamp_parser("2010-7-20 20:00"), std::runtime_error);
}

TEST(DefaultTimestampParser, ThrowsOnWrongSeparators) {
  EXPECT_THROW(TEM::default_timestamp_parser("2010/07/20 20:00"), std::runtime_error);
}

TEST(DefaultTimestampParser, ThrowsOnInvalidMonth) {
  EXPECT_THROW(TEM::default_timestamp_parser("2010-13-20 20:00"), std::invalid_argument);
}

TEST(DefaultTimestampParser, ThrowsOnInvalidDay) {
  EXPECT_THROW(TEM::default_timestamp_parser("2010-02-30 20:00"), std::invalid_argument);
}

TEST(DefaultTimestampParser, ThrowsOnInvalidHour) {
  EXPECT_THROW(TEM::default_timestamp_parser("2010-07-20 24:00"), std::invalid_argument);
}

TEST(DefaultTimestampParser, ThrowsOnInvalidMinute) {
  EXPECT_THROW(TEM::default_timestamp_parser("2010-07-20 20:60"), std::invalid_argument);
}

TEST(DefaultTimestampParser, ThrowsOnNonNumericFields) {
  EXPECT_THROW(TEM::default_timestamp_parser("abcd-ef-gh ij:kl"), std::invalid_argument);
}

// =============================================================================
// ColumnMapping::is_in_bounds tests
// =============================================================================

TEST(ColumnMappingIsInBounds, DefaultMappingIsInBounds) {
  TEM::ColumnMapping mapping{.timestamp{0}, .open{1}, .high{2}, .low{3}, .close{4}, .volume{5}};
  EXPECT_TRUE(mapping.is_in_bounds(6));
}

TEST(ColumnMappingIsInBounds, BoundaryIndexIsOutOfBounds) {
  TEM::ColumnMapping mapping{.timestamp{0}, .open{1}, .high{2}, .low{3}, .close{4}, .volume{5}};
  EXPECT_FALSE(mapping.is_in_bounds(5));
}

TEST(ColumnMappingIsInBounds, TimestampTooLargeIsOutOfBounds) {
  TEM::ColumnMapping mapping{.timestamp{6}, .open{1}, .high{2}, .low{3}, .close{1}, .volume{}};
  EXPECT_FALSE(mapping.is_in_bounds(6));
}

TEST(ColumnMappingIsInBounds, WithoutVolumeAllowsSmallerRow) {
  TEM::ColumnMapping mapping{.timestamp{0}, .open{1}, .high{2}, .low{3}, .close{4}, .volume{}};
  EXPECT_TRUE(mapping.is_in_bounds(5));
  EXPECT_FALSE(mapping.is_in_bounds(4));
}

TEST(ColumnMappingIsInBounds, OptionalVolumeCheckedOnlyWhenSet) {
  TEM::ColumnMapping with_volume{.timestamp{0}, .open{1}, .high{2}, .low{3}, .close{4}, .volume{4}};
  EXPECT_TRUE(with_volume.is_in_bounds(5));

  TEM::ColumnMapping volume_out{.timestamp{0}, .open{1}, .high{2}, .low{3}, .close{4}, .volume{5}};
  EXPECT_FALSE(volume_out.is_in_bounds(5));
}

// =============================================================================
// Death tests: an error escaping the process-level exception barrier (frame)
// is fatal. These verify that corrupt input kills the process when the
// consumer provides no catch.
// =============================================================================

TEST(CSVProviderDeathTest, MissingFileErrorEscapingBarrierIsFatal) {
  EXPECT_DEATH(
      [] {
        try {
          CSVImporterConfiguration config;
          config.path = std::filesystem::temp_directory_path() / "no_such_csv_123_file.csv";
          CSVProvider provider{std::move(config)};
          (void)provider;
        } catch (...) {
          std::abort();
        }
        std::abort();
      }(),
      "");
}

TEST(CSVProviderDeathTest, MalformedRowErrorEscapingBarrierIsFatal) {
  EXPECT_DEATH(
      [] {
        try {
          TempFile file{std::string{CSV_HEADER} + "2010-07-24 20:00,abc,1.1,0.9,1.05,100\n"};
          CSVProvider provider{make_config(file)};
          while (provider.get_next().has_value()) {
          }
        } catch (...) {
          std::abort();
        }
        std::abort();
      }(),
      "");
}

TEST(CSVProviderDeathTest, IteratorMisuseErrorEscapingBarrierIsFatal) {
  EXPECT_DEATH(
      [] {
        try {
          CSVProvider::Iterator end;
          ++end;
        } catch (...) {
          std::abort();
        }
        std::abort();
      }(),
      "");
}