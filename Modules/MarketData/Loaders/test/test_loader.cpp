#include <MarketData/CSVProvider.hpp>
#include <MarketData/loader.hpp>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace TEC = TradingEngine::Core;
namespace TEM = TradingEngine::MarketData;

namespace {

using Candle = TEM::Candle;
using CSVProvider = TEM::CSVProvider;
using CSVImporterConfiguration = TEM::CSVImporterConfiguration;

TEC::Timestamp make_ts(int y, int m, int d) {
  std::chrono::sys_days days = std::chrono::sys_days{std::chrono::year_month_day{
      std::chrono::year{y}, std::chrono::month{static_cast<unsigned>(m)},
      std::chrono::day{static_cast<unsigned>(d)}}};
  return days;
}

Candle make_candle(double open, double high, double low, double close, TEC::Timestamp ts,
                   std::optional<int> volume = std::nullopt) {
  return Candle{open, high, low, close, ts, volume};
}

struct TempFile {
  std::filesystem::path path;

  explicit TempFile(const std::string &content) {
    static std::atomic<unsigned> counter{0};
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    auto name = "loader_test_" + std::to_string(now) + "_" + std::to_string(::getpid()) + "_" +
                std::to_string(++counter) + ".csv";
    path = std::filesystem::temp_directory_path() / name;
    std::ofstream ofs(path);
    ofs << content;
  }

  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path, ec);
  }
};

// Range-compliant provider that keeps candles in memory and exposes the same
// get_next() based interface as real providers.
class FakeProvider {
public:
  class Iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Candle;
    using difference_type = std::ptrdiff_t;
    using pointer = const Candle *;
    using reference = const Candle &;

    Iterator() = default;
    explicit Iterator(FakeProvider &provider) : provider{&provider}, current{provider.get_next()} {}

    reference operator*() const noexcept { return *current; };
    pointer operator->() const noexcept { return &*current; };

    Iterator &operator++() {
      if (!provider) {
        throw std::logic_error("Incrementing end iterator");
      }
      current = provider->get_next();
      return *this;
    };

    Iterator operator++(int) {
      if (!provider) {
        throw std::logic_error("Incrementing end iterator");
      }
      auto temp{*this};
      current = provider->get_next();
      return temp;
    };

    bool operator==(std::default_sentinel_t) const noexcept {
      return (provider == nullptr || !current.has_value());
    }
    bool operator!=(std::default_sentinel_t s) const noexcept { return !(*this == s); }

  private:
    FakeProvider *provider{nullptr};
    std::optional<Candle> current{std::nullopt};
  };

  using iterator = Iterator;
  using sentinel = std::default_sentinel_t;

  explicit FakeProvider(std::vector<Candle> candles) : candles{std::move(candles)} {}

  std::optional<Candle> get_next() {
    if (position >= candles.size()) {
      return std::nullopt;
    }
    return candles[position++];
  }

  iterator begin() { return Iterator(*this); }
  sentinel end() noexcept { return std::default_sentinel; }

private:
  std::vector<Candle> candles;
  std::size_t position{0};
};

inline bool operator==(std::default_sentinel_t s, const FakeProvider::Iterator &it) noexcept {
  return it == s;
}
inline bool operator!=(std::default_sentinel_t s, const FakeProvider::Iterator &it) noexcept {
  return it != s;
}

// Provider that fails as soon as it is asked for the next candle.
class ThrowingProvider {
public:
  class Iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Candle;
    using difference_type = std::ptrdiff_t;
    using pointer = const Candle *;
    using reference = const Candle &;

    Iterator() = default;
    explicit Iterator(ThrowingProvider &provider)
        : provider{&provider}, current{provider.get_next()} {}

    reference operator*() const noexcept { return *current; };
    pointer operator->() const noexcept { return &*current; };

    Iterator &operator++() {
      if (!provider) {
        throw std::logic_error("Incrementing end iterator");
      }
      current = provider->get_next();
      return *this;
    };

    Iterator operator++(int) {
      if (!provider) {
        throw std::logic_error("Incrementing end iterator");
      }
      auto temp{*this};
      current = provider->get_next();
      return temp;
    };

    bool operator==(std::default_sentinel_t) const noexcept {
      return (provider == nullptr || !current.has_value());
    }
    bool operator!=(std::default_sentinel_t s) const noexcept { return !(*this == s); }

  private:
    ThrowingProvider *provider{nullptr};
    std::optional<Candle> current{std::nullopt};
  };

  using iterator = Iterator;
  using sentinel = std::default_sentinel_t;

  std::optional<Candle> get_next() { throw std::runtime_error("provider failure"); }

  iterator begin() { return Iterator(*this); }
  sentinel end() noexcept { return std::default_sentinel; }
};

inline bool operator==(std::default_sentinel_t s, const ThrowingProvider::Iterator &it) noexcept {
  return it == s;
}
inline bool operator!=(std::default_sentinel_t s, const ThrowingProvider::Iterator &it) noexcept {
  return it != s;
}

} // namespace

// =============================================================================
// Concept satisfaction tests
// =============================================================================

static_assert(TEM::HistoricalProvider<FakeProvider>);
static_assert(TEM::HistoricalProvider<CSVProvider>);
static_assert(!TEM::HistoricalProvider<std::vector<Candle>>);
static_assert(!TEM::HistoricalProvider<int>);

static_assert(TEM::HistoricalRange<FakeProvider>);
static_assert(TEM::HistoricalRange<CSVProvider>);
static_assert(TEM::HistoricalRange<std::vector<Candle>>);
static_assert(!TEM::HistoricalRange<int>);

// =============================================================================
// load_series from plain containers
// =============================================================================

TEST(LoadSeries, LoadsAllCandlesFromContainer) {
  std::vector<Candle> source{
      make_candle(1.0, 1.1, 0.9, 1.05, make_ts(2024, 1, 1), 100),
      make_candle(1.1, 1.2, 1.0, 1.15, make_ts(2024, 1, 2), 200),
      make_candle(1.15, 1.3, 1.05, 1.25, make_ts(2024, 1, 3), 300),
  };

  auto series = TEM::load_series(source);
  ASSERT_EQ(series.size(), 3u);
  EXPECT_DOUBLE_EQ(series[0].open, 1.0);
  EXPECT_DOUBLE_EQ(series[0].close, 1.05);
  EXPECT_EQ(series[0].volume, std::optional<int>(100));
  EXPECT_EQ(series[1].timestamp, make_ts(2024, 1, 2));
  EXPECT_DOUBLE_EQ(series[2].high, 1.3);
  EXPECT_DOUBLE_EQ(series[2].low, 1.05);
  EXPECT_DOUBLE_EQ(series[2].close, 1.25);
}

TEST(LoadSeries, PreservesOrder) {
  std::vector<Candle> source{
      make_candle(1.0, 1.1, 0.9, 1.05, make_ts(2024, 1, 1)),
      make_candle(1.2, 1.3, 1.1, 1.25, make_ts(2024, 1, 2)),
  };

  auto series = TEM::load_series(source);
  ASSERT_EQ(series.size(), 2u);
  EXPECT_EQ(series[0].timestamp, make_ts(2024, 1, 1));
  EXPECT_EQ(series[1].timestamp, make_ts(2024, 1, 2));
}

TEST(LoadSeries, EmptyContainerProducesEmptySeries) {
  std::vector<Candle> source;
  auto series = TEM::load_series(source);
  EXPECT_TRUE(series.empty());
  EXPECT_EQ(series.size(), 0u);
}

TEST(LoadSeries, PreservesGivenTimeFrame) {
  std::vector<Candle> source{make_candle(1.0, 1.1, 0.9, 1.05, make_ts(2024, 1, 1))};
  auto series = TEM::load_series(source, TEM::TimeFrame::H1);
  EXPECT_EQ(series.get_time_frame(), TEM::TimeFrame::H1);
}

TEST(LoadSeries, DefaultsToUnknownTimeFrame) {
  std::vector<Candle> source{make_candle(1.0, 1.1, 0.9, 1.05, make_ts(2024, 1, 1))};
  auto series = TEM::load_series(source);
  EXPECT_EQ(series.get_time_frame(), TEM::TimeFrame::Unknown);
}

// =============================================================================
// load_series from providers
// =============================================================================

TEST(LoadSeries, LoadsFromFakeProvider) {
  std::vector<Candle> source{
      make_candle(1.0, 1.1, 0.9, 1.05, make_ts(2024, 1, 1), 100),
      make_candle(1.1, 1.2, 1.0, 1.15, make_ts(2024, 1, 2), 200),
  };
  FakeProvider provider{source};

  auto series = TEM::load_series(provider);
  ASSERT_EQ(series.size(), 2u);
  EXPECT_EQ(series[0].timestamp, make_ts(2024, 1, 1));
  EXPECT_EQ(series[1].timestamp, make_ts(2024, 1, 2));
  EXPECT_EQ(series[0].volume, std::optional<int>(100));
}

TEST(LoadSeries, EmptyFakeProviderProducesEmptySeries) {
  FakeProvider provider{std::vector<Candle>{}};
  auto series = TEM::load_series(provider);
  EXPECT_TRUE(series.empty());
}

TEST(LoadSeries, LoadsFromCsvProvider) {
  TempFile file{
      "timestamp,open,high,low,close,volume\n"
      "2010-07-24 20:00,1.0,1.1,0.9,1.05,100\n"
      "2010-07-24 20:01,1.2,1.3,1.1,1.25,200\n"};
  CSVImporterConfiguration config;
  config.path = file.path;
  CSVProvider provider{std::move(config)};

  auto series = TEM::load_series(provider);
  ASSERT_EQ(series.size(), 2u);
  EXPECT_EQ(series[0].timestamp, TEC::make_timestamp(2010, 7, 24, 20, 0));
  EXPECT_DOUBLE_EQ(series[0].close, 1.05);
  EXPECT_EQ(series[0].volume, std::optional<int>(100));
  EXPECT_EQ(series[1].timestamp, TEC::make_timestamp(2010, 7, 24, 20, 1));
}

TEST(LoadSeries, ConsumesProviderExactlyLikeGetNext) {
  TempFile file{
      "timestamp,open,high,low,close,volume\n"
      "2010-07-24 20:00,1.0,1.1,0.9,1.05,100\n"
      "2010-07-24 20:01,1.2,1.3,1.1,1.25,200\n"};

  CSVImporterConfiguration config;
  config.path = file.path;
  CSVProvider ranged{config};
  auto series = TEM::load_series(ranged);

  CSVProvider manual{config};
  auto first = manual.get_next();
  auto second = manual.get_next();
  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());
  ASSERT_EQ(series.size(), 2u);
  EXPECT_EQ(series[0].timestamp, first->timestamp);
  EXPECT_DOUBLE_EQ(series[0].close, first->close);
  EXPECT_EQ(series[1].timestamp, second->timestamp);
  EXPECT_DOUBLE_EQ(series[1].close, second->close);
  EXPECT_FALSE(manual.get_next().has_value());
}

// =============================================================================
// load_series error propagation
// =============================================================================

TEST(LoadSeries, PropagatesProviderErrors) {
  ThrowingProvider provider;
  EXPECT_THROW(static_cast<void>(TEM::load_series(provider)), std::runtime_error);
}

// =============================================================================
// Death tests: an error escaping the process-level exception barrier (frame)
// is fatal. These verify that corrupt provider input kills the process when
// the consumer provides no catch.
// =============================================================================

TEST(LoadSeriesDeathTest, ProviderErrorEscapingBarrierIsFatal) {
  EXPECT_DEATH(
      [] {
        try {
          ThrowingProvider provider;
          static_cast<void>(TEM::load_series(provider));
        } catch (...) {
          std::abort();
        }
        std::abort();
      }(),
      "");
}