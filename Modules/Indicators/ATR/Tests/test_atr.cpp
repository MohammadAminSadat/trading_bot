#include <Core/Core.hpp>
#include <Indicators/ATR.hpp>
#include <chrono>
#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>

using namespace TradingEngine::Core;
using namespace TradingEngine::Indicators;
using namespace TradingEngine::MarketData;

namespace {

Timestamp make_ts(int y, int m, int d) {
  std::chrono::sys_days days = std::chrono::sys_days{std::chrono::year_month_day{
      std::chrono::year{y}, std::chrono::month{static_cast<unsigned>(m)},
      std::chrono::day{static_cast<unsigned>(d)}}};
  return days;
}

Candle make_candle(double o, double h, double l, double c, int y = 2024, int m = 1, int d = 1) {
  return Candle{o, h, l, c, make_ts(y, m, d)};
}

} // namespace

// =============================================================================
// Constructor tests
// =============================================================================

TEST(ATRConstructorTest, DefaultPeriodCreatesValidIndicator) {
  EXPECT_NO_THROW(ATR atr(14));
}

TEST(ATRConstructorTest, CustomPeriodCreatesValidIndicator) {
  EXPECT_NO_THROW(ATR atr(5));
  EXPECT_NO_THROW(ATR atr(20));
}

TEST(ATRConstructorTest, PeriodOneCreatesValidIndicator) {
  EXPECT_NO_THROW(ATR atr(1));
}

TEST(ATRConstructorTest, ZeroPeriodThrowsInvalidArgument) {
  EXPECT_THROW(ATR atr(0), std::invalid_argument);
}

TEST(ATRConstructorTest, ZeroPeriodDeathTest) {
  EXPECT_DEATH(
      {
        try {
          ATR(0);
        } catch (const std::exception &) {
          std::abort();
        }
      },
      "");
}

// =============================================================================
// Signal type and requirements tests
// =============================================================================

TEST(ATRSignalTypeTest, ReturnsZeroCross) {
  ATR atr(14);
  EXPECT_EQ(atr.signal_type(), SignalType::ZeroCross);
}

TEST(ATRRequirementsTest, RequiresCloseHighLow) {
  ATR atr(14);
  const auto &req = atr.requirements();
  ASSERT_EQ(req.size(), 3);
  EXPECT_EQ(req[0], InputData::Close);
  EXPECT_EQ(req[1], InputData::High);
  EXPECT_EQ(req[2], InputData::Low);
}

// =============================================================================
// Warmup period tests
// =============================================================================

TEST(ATRWarmupTest, FirstUpdateReturnsNullopt) {
  ATR atr(3);
  EXPECT_FALSE(atr.update(100.0, 110.0, 95.0));
}

TEST(ATRWarmupTest, FirstCandleUpdateReturnsNullopt) {
  ATR atr(3);
  auto c = make_candle(100.0, 110.0, 95.0, 105.0);
  EXPECT_FALSE(atr.update(c));
}

TEST(ATRWarmupTest, ReturnsNulloptDuringWarmup) {
  ATR atr(3);
  atr.update(100.0, 110.0, 90.0);
  EXPECT_FALSE(atr.update(105.0, 115.0, 100.0));
  EXPECT_FALSE(atr.update(110.0, 120.0, 105.0));
}

TEST(ATRWarmupTest, ReturnsValueWhenWarmupComplete) {
  ATR atr(3);
  atr.update(100.0, 110.0, 90.0);
  atr.update(105.0, 115.0, 100.0);
  atr.update(110.0, 120.0, 105.0);
  const auto result = atr.update(115.0, 125.0, 110.0);
  ASSERT_TRUE(result.has_value());
}

TEST(ATRWarmupTest, PeriodOneReturnsValueOnSecondUpdate) {
  ATR atr(1);
  atr.update(100.0, 110.0, 90.0);
  const auto result = atr.update(105.0, 115.0, 100.0);
  ASSERT_TRUE(result.has_value());
}

// =============================================================================
// True Range computation tests
// =============================================================================

TEST(ATRComputationTest, ComputesTrueRangeCorrectly) {
  ATR atr(2);
  atr.update(100.0, 105.0, 90.0);
  atr.update(102.0, 110.0, 95.0);
  const auto result = atr.update(108.0, 115.0, 100.0);
  ASSERT_TRUE(result.has_value());
  double tr2 = std::max({std::abs(110.0 - 95.0), std::abs(102.0 - 110.0), std::abs(102.0 - 95.0)});
  double tr3 =
      std::max({std::abs(115.0 - 100.0), std::abs(108.0 - 115.0), std::abs(108.0 - 100.0)});
  double expected = (tr2 + tr3) / 2.0;
  EXPECT_DOUBLE_EQ(result->signal1, expected);
}

TEST(ATRComputationTest, Signal2IsZero) {
  ATR atr(1);
  atr.update(100.0, 110.0, 90.0);
  const auto result = atr.update(105.0, 115.0, 100.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal2, 0.0);
}

// =============================================================================
// True Range edge cases
// =============================================================================

TEST(ATRTrueRangeTest, HighMinusLowIsLargest) {
  ATR atr(1);
  atr.update(100.0, 100.0, 100.0);
  const auto result = atr.update(100.0, 120.0, 95.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 25.0);
}

TEST(ATRTrueRangeTest, PrevCloseMinusHighIsLargest) {
  ATR atr(1);
  atr.update(130.0, 130.0, 130.0);
  const auto result = atr.update(100.0, 120.0, 100.0);
  ASSERT_TRUE(result.has_value());
  double tr = std::max({std::abs(120.0 - 100.0), std::abs(130.0 - 120.0), std::abs(130.0 - 100.0)});
  EXPECT_DOUBLE_EQ(result->signal1, tr);
}

TEST(ATRTrueRangeTest, PrevCloseMinusLowIsLargest) {
  ATR atr(1);
  atr.update(95.0, 95.0, 95.0);
  const auto result = atr.update(100.0, 110.0, 85.0);
  ASSERT_TRUE(result.has_value());
  double tr = std::max({std::abs(110.0 - 85.0), std::abs(95.0 - 110.0), std::abs(95.0 - 85.0)});
  EXPECT_DOUBLE_EQ(result->signal1, tr);
}

// =============================================================================
// Rolling window tests
// =============================================================================

TEST(ATRRollingTest, RollingWindowUpdatesCorrectly) {
  ATR atr(3);
  atr.update(100.0, 105.0, 95.0);
  atr.update(102.0, 108.0, 98.0);
  atr.update(108.0, 112.0, 100.0);
  auto r1 = atr.update(110.0, 115.0, 105.0);
  ASSERT_TRUE(r1.has_value());

  auto r2 = atr.update(112.0, 118.0, 108.0);
  ASSERT_TRUE(r2.has_value());

  EXPECT_FALSE(std::isnan(r1->signal1));
  EXPECT_FALSE(std::isnan(r2->signal1));
}

TEST(ATRRollingTest, RemembersPreviousClose) {
  ATR atr(2);
  atr.update(100.0, 110.0, 90.0);
  atr.update(105.0, 115.0, 100.0);
  const auto result = atr.update(108.0, 120.0, 95.0);
  ASSERT_TRUE(result.has_value());
  double tr = std::max({std::abs(120.0 - 95.0), std::abs(108.0 - 120.0), std::abs(108.0 - 95.0)});
  double prev =
      std::max({std::abs(115.0 - 100.0), std::abs(105.0 - 115.0), std::abs(105.0 - 100.0)});
  double expected = (tr + prev) / 2.0;
  EXPECT_DOUBLE_EQ(result->signal1, expected);
}

// =============================================================================
// Candle integration tests
// =============================================================================

TEST(ATRCandleTest, SetsTimestampCorrectly) {
  ATR atr(1);
  atr.update(make_candle(100.0, 110.0, 90.0, 105.0));
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 115.0, 95.0, 110.0, ts};
  const auto result = atr.update(c);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->timestamp, ts);
}

TEST(ATRCandleTest, SetsIndicatorId) {
  ATR atr(1);
  atr.update(make_candle(100.0, 110.0, 90.0, 105.0));
  const auto result = atr.update(make_candle(100.0, 115.0, 95.0, 110.0));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, atr.indicator_id());
}

// =============================================================================
// Edge case / boundary tests
// =============================================================================

TEST(ATREdgeTest, ZeroPrices) {
  ATR atr(2);
  atr.update(0.0, 0.0, 0.0);
  atr.update(0.0, 0.0, 0.0);
  const auto result = atr.update(0.0, 0.0, 0.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 0.0);
}

TEST(ATREdgeTest, NegativePrices) {
  ATR atr(1);
  atr.update(-5.0, -3.0, -10.0);
  const auto result = atr.update(-8.0, -2.0, -12.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_GT(result->signal1, 0.0);
}

TEST(ATREdgeTest, LargePriceSwings) {
  ATR atr(1);
  atr.update(100.0, 100.0, 100.0);
  const auto result = atr.update(100.0, 200.0, 50.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 150.0);
}

TEST(ATREdgeTest, LargePeriod) {
  ATR atr(100);
  for (int i = 0; i < 100; ++i) {
    atr.update(100.0 + i, 110.0 + i, 90.0 + i);
  }
  const auto result = atr.update(200.0, 210.0, 190.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(std::isnan(result->signal1));
}
