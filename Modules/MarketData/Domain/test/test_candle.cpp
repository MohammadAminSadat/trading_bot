#include <chrono>
#include <cmath>
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>
#include <optional>
#include <vector>

#include "MarketData/candle.hpp"

using namespace TradingEngine::Core;
using namespace TradingEngine::MarketData;

namespace {

Timestamp make_ts(int y, int m, int d) {
  std::chrono::sys_days days = std::chrono::sys_days{std::chrono::year_month_day{
      std::chrono::year{y}, std::chrono::month{static_cast<unsigned>(m)},
      std::chrono::day{static_cast<unsigned>(d)}}};
  return days;
}

} // namespace

// =============================================================================
// TimeFrame enum tests
// =============================================================================

TEST(TimeFrameTest, AllValuesAreDistinct) {
  EXPECT_NE(static_cast<int>(TimeFrame::Unknown), static_cast<int>(TimeFrame::M10));
  EXPECT_NE(static_cast<int>(TimeFrame::M10), static_cast<int>(TimeFrame::M30));
  EXPECT_NE(static_cast<int>(TimeFrame::M30), static_cast<int>(TimeFrame::H1));
  EXPECT_NE(static_cast<int>(TimeFrame::H1), static_cast<int>(TimeFrame::H2));
  EXPECT_NE(static_cast<int>(TimeFrame::H2), static_cast<int>(TimeFrame::H4));
  EXPECT_NE(static_cast<int>(TimeFrame::H4), static_cast<int>(TimeFrame::H12));
  EXPECT_NE(static_cast<int>(TimeFrame::H12), static_cast<int>(TimeFrame::D1));
  EXPECT_NE(static_cast<int>(TimeFrame::D1), static_cast<int>(TimeFrame::W1));
}

// =============================================================================
// Candle constructor tests
// =============================================================================

TEST(CandleConstructorTest, StoresAllFieldsCorrectly) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.5, 105.0, 99.0, 103.2, ts, 1000};

  EXPECT_DOUBLE_EQ(c.open, 100.5);
  EXPECT_DOUBLE_EQ(c.high, 105.0);
  EXPECT_DOUBLE_EQ(c.low, 99.0);
  EXPECT_DOUBLE_EQ(c.close, 103.2);
  EXPECT_EQ(c.volume, 1000);
  EXPECT_EQ(c.timestamp, ts);
}

TEST(CandleConstructorTest, ZeroVolume) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{10.0, 10.0, 10.0, 10.0, ts, 0};
  EXPECT_EQ(c.volume, 0);
}

TEST(CandleConstructorTest, NegativePrices) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{-5.0, 0.0, -10.0, -2.0, ts, 50};
  EXPECT_DOUBLE_EQ(c.open, -5.0);
  EXPECT_DOUBLE_EQ(c.high, 0.0);
  EXPECT_DOUBLE_EQ(c.low, -10.0);
  EXPECT_DOUBLE_EQ(c.close, -2.0);
}

TEST(CandleConstructorTest, DefaultVolumeIsNullopt) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.5, 105.0, 99.0, 103.2, ts};
  EXPECT_FALSE(c.volume.has_value());
}

TEST(CandleConstructorTest, ExplicitVolumeIsEngaged) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.5, 105.0, 99.0, 103.2, ts, 500};
  EXPECT_TRUE(c.volume.has_value());
  EXPECT_EQ(c.volume, 500);
}

// =============================================================================
// Candle::validate() tests
// =============================================================================

TEST(CandleValidateTest, ValidCandle) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 110.0, 90.0, 105.0, ts, 500};
  EXPECT_TRUE(c.validate());
}

TEST(CandleValidateTest, AllPricesEqual) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{42.0, 42.0, 42.0, 42.0, ts, 100};
  EXPECT_TRUE(c.validate());
}

TEST(CandleValidateTest, VolumeZeroIsValid) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 110.0, 90.0, 105.0, ts, 0};
  EXPECT_TRUE(c.validate());
}

TEST(CandleValidateTest, NullVolumeIsValid) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 110.0, 90.0, 105.0, ts};
  EXPECT_TRUE(c.validate());
  EXPECT_FALSE(c.volume.has_value());
}

TEST(CandleValidateTest, HighLessThanOpen) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 95.0, 90.0, 98.0, ts, 100};
  EXPECT_FALSE(c.validate());
}

TEST(CandleValidateTest, HighLessThanClose) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 95.0, 90.0, 98.0, ts, 100};
  EXPECT_FALSE(c.validate());
}

TEST(CandleValidateTest, HighLessThanLow) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 80.0, 90.0, 100.0, ts, 100};
  EXPECT_FALSE(c.validate());
}

TEST(CandleValidateTest, LowGreaterThanClose) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 110.0, 105.0, 100.0, ts, 100};
  EXPECT_FALSE(c.validate());
}

TEST(CandleValidateTest, LowGreaterThanOpen) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 110.0, 105.0, 108.0, ts, 100};
  EXPECT_FALSE(c.validate());
}

TEST(CandleValidateTest, NegativeVolume) {
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 110.0, 90.0, 105.0, ts, -1};
  EXPECT_FALSE(c.validate());
}

TEST(CandleValidateTest, DefaultZeroValues) {
  auto ts = std::chrono::sys_seconds{};
  Candle c{0.0, 0.0, 0.0, 0.0, ts};
  EXPECT_TRUE(c.validate());
}

// =============================================================================
// Candle::is_bullish() tests
// =============================================================================

TEST(CandleIsBullishTest, CloseGreaterThanOpen) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 110.0, 90.0, 105.0, ts, 100};
  EXPECT_TRUE(c.is_bullish());
}

TEST(CandleIsBullishTest, CloseEqualToOpen) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 110.0, 90.0, 100.0, ts, 100};
  EXPECT_TRUE(c.is_bullish());
}

TEST(CandleIsBullishTest, CloseLessThanOpen) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 110.0, 90.0, 95.0, ts, 100};
  EXPECT_FALSE(c.is_bullish());
}

TEST(CandleIsBullishTest, NegativePricesBullish) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{-10.0, -5.0, -15.0, -8.0, ts, 100};
  EXPECT_TRUE(c.is_bullish());
}

// =============================================================================
// Candle::is_bearish() tests
// =============================================================================

TEST(CandleIsBearishTest, CloseLessThanOpen) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 110.0, 90.0, 95.0, ts, 100};
  EXPECT_TRUE(c.is_bearish());
}

TEST(CandleIsBearishTest, CloseEqualToOpen) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 110.0, 90.0, 100.0, ts, 100};
  EXPECT_FALSE(c.is_bearish());
}

TEST(CandleIsBearishTest, CloseGreaterThanOpen) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 110.0, 90.0, 105.0, ts, 100};
  EXPECT_FALSE(c.is_bearish());
}

TEST(CandleIsBearishTest, NegativePricesBearish) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{-8.0, -5.0, -15.0, -10.0, ts, 100};
  EXPECT_TRUE(c.is_bearish());
}

// =============================================================================
// Candle::typical_price() tests
// =============================================================================

TEST(CandleTypicalPriceTest, BasicCalculation) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 120.0, 90.0, 105.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.typical_price(), (90.0 + 120.0 + 105.0) / 3.0);
}

TEST(CandleTypicalPriceTest, AllEqual) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{50.0, 50.0, 50.0, 50.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.typical_price(), 50.0);
}

TEST(CandleTypicalPriceTest, ZeroValues) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{0.0, 0.0, 0.0, 0.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.typical_price(), 0.0);
}

TEST(CandleTypicalPriceTest, LargeValues) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{0.0, 1e9, 0.0, 0.5e9, ts, 100};
  EXPECT_DOUBLE_EQ(c.typical_price(), (0.0 + 1e9 + 0.5e9) / 3.0);
}

// =============================================================================
// Candle::median_price() tests
// =============================================================================

TEST(CandleMedianPriceTest, BasicCalculation) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 120.0, 90.0, 105.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.median_price(), (90.0 + 120.0) / 2.0);
}

TEST(CandleMedianPriceTest, HighEqualsLow) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{10.0, 50.0, 50.0, 30.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.median_price(), 50.0);
}

TEST(CandleMedianPriceTest, ZeroValues) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{0.0, 0.0, 0.0, 0.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.median_price(), 0.0);
}

TEST(CandleMedianPriceTest, NegativeValues) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{0.0, -5.0, -15.0, 0.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.median_price(), (-15.0 + -5.0) / 2.0);
}

// =============================================================================
// Candle::weighted_close_price() tests
// =============================================================================

TEST(CandleWeightedClosePriceTest, BasicCalculation) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 120.0, 90.0, 105.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.weighted_close_price(), (90.0 + 120.0 + 2.0 * 105.0) / 4.0);
}

TEST(CandleWeightedClosePriceTest, AllEqual) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 100.0, 100.0, 100.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.weighted_close_price(), 100.0);
}

TEST(CandleWeightedClosePriceTest, CloseDominates) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{0.0, 100.0, 100.0, 100.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.weighted_close_price(), (100.0 + 100.0 + 2.0 * 100.0) / 4.0);
}

TEST(CandleWeightedClosePriceTest, CloseIsZero) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{0.0, 100.0, 80.0, 0.0, ts, 100};
  EXPECT_DOUBLE_EQ(c.weighted_close_price(), (80.0 + 100.0 + 2.0 * 0.0) / 4.0);
}

// =============================================================================
// CandleSeries with different TimeFrame values
// =============================================================================

TEST(CandleSeriesTimeFrameTest, EachTimeFrame) {
  auto ts = make_ts(2024, 1, 1);

  for (auto tf : {TimeFrame::Unknown, TimeFrame::M10, TimeFrame::M30, TimeFrame::H1, TimeFrame::H2,
                  TimeFrame::H4, TimeFrame::H12, TimeFrame::D1, TimeFrame::W1}) {
    CandleSeries series{std::vector<Candle>{}, tf};
    series.emplace_back(10.0, 20.0, 5.0, 15.0, ts);
    EXPECT_EQ(series.size(), 1);
    EXPECT_TRUE(series[0].validate());
  }
}

// =============================================================================
// CandleSeries constructor tests
// =============================================================================

TEST(CandleSeriesConstructorTest, EmptySeries) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::H1};
  EXPECT_TRUE(series.empty());
  EXPECT_EQ(series.size(), 0);
}

TEST(CandleSeriesConstructorTest, NonEmptySeries) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(10.0, 15.0, 8.0, 12.0, ts, 100);
  candles.emplace_back(12.0, 16.0, 10.0, 14.0, ts, 200);

  CandleSeries series{std::move(candles), TimeFrame::M10};
  EXPECT_FALSE(series.empty());
  EXPECT_EQ(series.size(), 2);
}

TEST(CandleSeriesConstructorTest, MoveSemanticsEmptiesSource) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(10.0, 15.0, 8.0, 12.0, ts, 100);

  CandleSeries series{std::move(candles), TimeFrame::H1};
  EXPECT_EQ(series.size(), 1);
}

// =============================================================================
// CandleSeries::size() and empty() tests
// =============================================================================

TEST(CandleSeriesSizeTest, InitiallyEmpty) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::H1};
  EXPECT_EQ(series.size(), 0);
  EXPECT_TRUE(series.empty());
}

TEST(CandleSeriesSizeTest, SizeAfterPushBack) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::H1};
  auto ts = make_ts(2024, 1, 1);
  series.push_back(Candle{10.0, 15.0, 8.0, 12.0, ts, 100});
  EXPECT_EQ(series.size(), 1);
  EXPECT_FALSE(series.empty());

  series.push_back(Candle{12.0, 16.0, 10.0, 14.0, ts, 200});
  EXPECT_EQ(series.size(), 2);
}

// =============================================================================
// CandleSeries::push_back() tests
// =============================================================================

TEST(CandleSeriesPushBackTest, PushedCandleIsRetrievable) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::D1};
  auto ts = make_ts(2024, 6, 15);
  series.push_back(Candle{100.0, 110.0, 90.0, 105.0, ts, 500});

  const auto& c = series[0];
  EXPECT_DOUBLE_EQ(c.open, 100.0);
  EXPECT_DOUBLE_EQ(c.high, 110.0);
  EXPECT_DOUBLE_EQ(c.low, 90.0);
  EXPECT_DOUBLE_EQ(c.close, 105.0);
  EXPECT_EQ(c.volume, 500);
  EXPECT_EQ(c.timestamp, ts);
}

TEST(CandleSeriesPushBackTest, MultiplePushBacks) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::M10};
  auto ts = make_ts(2024, 1, 1);

  for (int i = 0; i < 100; ++i) {
    series.push_back(Candle{double(i), double(i + 1), double(i - 1), double(i), ts, i});
  }
  EXPECT_EQ(series.size(), 100);
  EXPECT_DOUBLE_EQ(series[0].open, 0.0);
  EXPECT_DOUBLE_EQ(series[99].open, 99.0);
}

// =============================================================================
// CandleSeries::emplace_back() tests
// =============================================================================

TEST(CandleSeriesEmplaceBackTest, EmplacedCandleIsRetrievable) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::D1};
  auto ts = make_ts(2024, 6, 15);
  series.emplace_back(100.0, 110.0, 90.0, 105.0, ts, 500);

  const auto& c = series[0];
  EXPECT_DOUBLE_EQ(c.open, 100.0);
  EXPECT_DOUBLE_EQ(c.high, 110.0);
  EXPECT_DOUBLE_EQ(c.low, 90.0);
  EXPECT_DOUBLE_EQ(c.close, 105.0);
  EXPECT_EQ(c.volume, 500);
  EXPECT_EQ(c.timestamp, ts);
}

TEST(CandleSeriesEmplaceBackTest, DefaultVolume) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::H1};
  auto ts = make_ts(2024, 6, 15);
  series.emplace_back(1.0, 2.0, 0.5, 1.5, ts);

  EXPECT_EQ(series.size(), 1);
  EXPECT_FALSE(series[0].volume.has_value());
  EXPECT_TRUE(series[0].validate());
}

TEST(CandleSeriesEmplaceBackTest, MultipleEmplaceBacks) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::M10};
  auto ts = make_ts(2024, 1, 1);

  for (int i = 0; i < 100; ++i) {
    series.emplace_back(double(i), double(i + 1), double(i - 1), double(i), ts, i);
  }
  EXPECT_EQ(series.size(), 100);
  EXPECT_DOUBLE_EQ(series[0].open, 0.0);
  EXPECT_DOUBLE_EQ(series[99].open, 99.0);
}

// =============================================================================
// CandleSeries::operator[] tests
// =============================================================================

TEST(CandleSeriesOperatorBracketTest, AccessByIndex) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(10.0, 15.0, 8.0, 12.0, ts, 100);
  candles.emplace_back(20.0, 25.0, 18.0, 22.0, ts, 200);

  CandleSeries series{std::move(candles), TimeFrame::H1};

  EXPECT_DOUBLE_EQ(series[0].open, 10.0);
  EXPECT_DOUBLE_EQ(series[1].open, 20.0);
}

TEST(CandleSeriesOperatorBracketTest, ConstAccess) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(10.0, 15.0, 8.0, 12.0, ts, 100);
  candles.emplace_back(20.0, 25.0, 18.0, 22.0, ts, 200);

  const CandleSeries series{std::move(candles), TimeFrame::H1};

  EXPECT_DOUBLE_EQ(series[0].open, 10.0);
  EXPECT_DOUBLE_EQ(series[1].close, 22.0);
}

TEST(CandleSeriesOperatorBracketTest, ModifyViaNonConst) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(10.0, 15.0, 8.0, 12.0, ts, 100);

  CandleSeries series{std::move(candles), TimeFrame::M10};
  series[0].open = 50.0;
  series[0].volume = 999;

  EXPECT_DOUBLE_EQ(series[0].open, 50.0);
  EXPECT_EQ(series[0].volume, 999);
}

// =============================================================================
// CandleSeries::latest() tests
// =============================================================================

TEST(CandleSeriesLatestTest, LatestReturnsLastCandle) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::M10};
  auto ts = make_ts(2024, 1, 1);

  series.push_back(Candle{1.0, 2.0, 0.5, 1.5, ts, 10});
  EXPECT_DOUBLE_EQ(series.latest().close, 1.5);

  series.push_back(Candle{2.0, 3.0, 1.5, 2.5, ts, 20});
  EXPECT_DOUBLE_EQ(series.latest().close, 2.5);
  EXPECT_EQ(series.latest().volume, 20);
}

TEST(CandleSeriesLatestTest, LatestIsConstRef) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(10.0, 15.0, 8.0, 12.0, ts, 100);

  const CandleSeries series{std::move(candles), TimeFrame::H4};
  const Candle& latest = series.latest();
  EXPECT_DOUBLE_EQ(latest.open, 10.0);
  EXPECT_EQ(latest.volume, 100);
}

// =============================================================================
// CandleSeries::begin()/end() tests
// =============================================================================

TEST(CandleSeriesIterationTest, MutableIteration) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(10.0, 15.0, 8.0, 12.0, ts, 100);
  candles.emplace_back(20.0, 25.0, 18.0, 22.0, ts, 200);
  candles.emplace_back(30.0, 35.0, 28.0, 32.0, ts, 300);

  CandleSeries series{std::move(candles), TimeFrame::H1};

  double sum = 0.0;
  int count = 0;
  for (auto& c : series) {
    sum += c.open;
    ++count;
  }
  EXPECT_EQ(count, 3);
  EXPECT_DOUBLE_EQ(sum, 60.0);

  for (auto& c : series) {
    c.volume = 999;
  }
  EXPECT_EQ(series[0].volume, 999);
  EXPECT_EQ(series[1].volume, 999);
  EXPECT_EQ(series[2].volume, 999);
}

TEST(CandleSeriesIterationTest, ConstIteration) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(10.0, 15.0, 8.0, 12.0, ts, 100);
  candles.emplace_back(20.0, 25.0, 18.0, 22.0, ts, 200);

  const CandleSeries series{std::move(candles), TimeFrame::H1};

  double sum = 0.0;
  int count = 0;
  for (const auto& c : series) {
    sum += c.close;
    ++count;
  }
  EXPECT_EQ(count, 2);
  EXPECT_DOUBLE_EQ(sum, 34.0);
}

TEST(CandleSeriesIterationTest, EmptySeriesIteration) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::H1};
  int count = 0;
  for (const auto& c : series) {
    (void)c;
    ++count;
  }
  EXPECT_EQ(count, 0);

  EXPECT_EQ(series.begin(), series.end());
  EXPECT_EQ(static_cast<const CandleSeries&>(series).begin(),
            static_cast<const CandleSeries&>(series).end());
}

// =============================================================================
// CandleSeries::reserve() tests
// =============================================================================

TEST(CandleSeriesReserveTest, Reservations) {
  CandleSeries series{std::vector<Candle>{}, TimeFrame::M10};
  series.reserve(1000);

  auto ts = make_ts(2024, 1, 1);
  for (int i = 0; i < 1000; ++i) {
    series.push_back(Candle{double(i), double(i), double(i), double(i), ts, i});
  }
  EXPECT_EQ(series.size(), 1000);
}

// =============================================================================
// Integration: CandleSeries with validation
// =============================================================================

TEST(CandleSeriesIntegrationTest, ValidateAllCandles) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(100.0, 110.0, 90.0, 105.0, ts, 500);
  candles.emplace_back(105.0, 115.0, 100.0, 110.0, ts, 300);
  candles.emplace_back(110.0, 120.0, 105.0, 108.0, ts, 200);

  CandleSeries series{std::move(candles), TimeFrame::H1};

  for (const auto& c : series) {
    EXPECT_TRUE(c.validate());
  }
}

TEST(CandleSeriesIntegrationTest, MixedValidAndInvalid) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(100.0, 110.0, 90.0, 105.0, ts, 500);
  candles.emplace_back(100.0, 90.0, 110.0, 105.0, ts, 100);
  candles.emplace_back(110.0, 120.0, 105.0, 108.0, ts, -1);

  CandleSeries series{std::move(candles), TimeFrame::H1};

  EXPECT_TRUE(series[0].validate());
  EXPECT_FALSE(series[1].validate());
  EXPECT_FALSE(series[2].validate());
}

TEST(CandleSeriesIntegrationTest, BullishBearishDistribution) {
  auto ts = make_ts(2024, 1, 1);
  std::vector<Candle> candles;
  candles.emplace_back(100.0, 110.0, 90.0, 105.0, ts, 500);
  candles.emplace_back(105.0, 115.0, 100.0, 100.0, ts, 300);
  candles.emplace_back(100.0, 110.0, 90.0, 95.0, ts, 200);

  CandleSeries series{std::move(candles), TimeFrame::H1};

  EXPECT_TRUE(series[0].is_bullish());
  EXPECT_FALSE(series[1].is_bullish());
  EXPECT_FALSE(series[2].is_bullish());

  EXPECT_FALSE(series[0].is_bearish());
  EXPECT_TRUE(series[1].is_bearish());
  EXPECT_TRUE(series[2].is_bearish());
}

// =============================================================================
// Edge cases and boundary tests
// =============================================================================

TEST(CandleEdgeCaseTest, FloatingPointPrecision) {
  auto ts = make_ts(2024, 1, 1);
  double eps = std::numeric_limits<double>::epsilon();
  Candle c{1.0, 1.0 + eps, 1.0 - eps, 1.0, ts, 100};
  EXPECT_TRUE(c.validate());

  double tp = c.typical_price();
  EXPECT_GE(tp, 0.0);
}

TEST(CandleEdgeCaseTest, MaximumVolume) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 110.0, 90.0,
           105.0, ts,    static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())};
  EXPECT_TRUE(c.validate());
  EXPECT_EQ(c.volume, std::numeric_limits<std::int64_t>::max());
}

TEST(CandleEdgeCaseTest, DojiCandleAllSame) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{42.0, 42.0, 42.0, 42.0, ts, 100};
  EXPECT_TRUE(c.validate());
  EXPECT_TRUE(c.is_bullish());
  EXPECT_FALSE(c.is_bearish());
  EXPECT_DOUBLE_EQ(c.typical_price(), 42.0);
  EXPECT_DOUBLE_EQ(c.median_price(), 42.0);
  EXPECT_DOUBLE_EQ(c.weighted_close_price(), 42.0);
}

TEST(CandleEdgeCaseTest, MarubozuBullish) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{100.0, 120.0, 100.0, 120.0, ts, 1000};
  EXPECT_TRUE(c.validate());
  EXPECT_TRUE(c.is_bullish());
  EXPECT_FALSE(c.is_bearish());
}

TEST(CandleEdgeCaseTest, MarubozuBearish) {
  auto ts = make_ts(2024, 1, 1);
  Candle c{120.0, 120.0, 100.0, 100.0, ts, 1000};
  EXPECT_TRUE(c.validate());
  EXPECT_FALSE(c.is_bullish());
  EXPECT_TRUE(c.is_bearish());
}
