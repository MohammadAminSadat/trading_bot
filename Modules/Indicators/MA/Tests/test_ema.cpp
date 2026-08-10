#include <Core/Core.hpp>
#include <Indicators/EMA.hpp>
#include <chrono>
#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>
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

Candle make_candle(double close, int y = 2024, int m = 1, int d = 1) {
  return Candle{close, close, close, close, make_ts(y, m, d)};
}

} // namespace

// =============================================================================
// Constructor tests
// =============================================================================

TEST(EMAConstructorTest, DefaultSmoothingFactor) {
  EXPECT_NO_THROW(EMA ema(14));
}

TEST(EMAConstructorTest, CustomSmoothingFactor) {
  EXPECT_NO_THROW(EMA ema(14, 1.5));
  EXPECT_NO_THROW(EMA ema(14, 3.0));
}

TEST(EMAConstructorTest, PeriodOneDefault) {
  EXPECT_NO_THROW(EMA ema(1));
}

TEST(EMAConstructorTest, PeriodOneCustomSmoothing) {
  EXPECT_NO_THROW(EMA ema(1, 2.5));
}

TEST(EMAConstructorTest, ZeroPeriodThrowsInvalidArgument) {
  EXPECT_THROW(EMA ema(0), std::invalid_argument);
}

TEST(EMAConstructorTest, ZeroPeriodWithSmoothingThrowsInvalidArgument) {
  EXPECT_THROW(EMA ema(0, 2.0), std::invalid_argument);
}

TEST(EMAConstructorTest, ZeroPeriodDeathTest) {
  EXPECT_DEATH(
      {
        try {
          EMA(0);
        } catch (const std::exception &) {
          std::abort();
        }
      },
      "");
}

// =============================================================================
// Signal type and requirements tests
// =============================================================================

TEST(EMASignalTypeTest, ReturnsPriceCross) {
  EMA ema(14);
  EXPECT_EQ(ema.signal_type(), SignalType::PriceCross);
}

TEST(EMARequirementsTest, RequiresCloseOnly) {
  EMA ema(14);
  const auto &req = ema.requirements();
  ASSERT_EQ(req.size(), 1);
  EXPECT_EQ(req[0], InputData::Close);
}

// =============================================================================
// Warmup period tests
// =============================================================================

TEST(EMAWarmupTest, FirstUpdateReturnsNullopt) {
  EMA ema(14);
  EXPECT_FALSE(ema.update(100.0));
}

TEST(EMAWarmupTest, SecondUpdateReturnsValue) {
  EMA ema(14);
  ema.update(100.0);
  const auto result = ema.update(110.0);
  ASSERT_TRUE(result.has_value());
}

TEST(EMAWarmupTest, FirstCandleUpdateReturnsNullopt) {
  EMA ema(14);
  EXPECT_FALSE(ema.update(make_candle(100.0)));
}

TEST(EMAWarmupTest, SecondCandleUpdateReturnsValue) {
  EMA ema(14);
  ema.update(make_candle(100.0));
  const auto result = ema.update(make_candle(110.0));
  ASSERT_TRUE(result.has_value());
}

// =============================================================================
// Computation tests
// =============================================================================

TEST(EMAComputationTest, ComputesCorrectEMA) {
  EMA ema(5, 2.0);
  ema.update(10.0);
  auto r1 = ema.update(12.0);
  ASSERT_TRUE(r1.has_value());
  double alpha = 2.0 / (1.0 + 5.0);
  double expected = 12.0 * alpha + 10.0 * (1.0 - alpha);
  EXPECT_DOUBLE_EQ(r1->signal1, expected);
}

TEST(EMAComputationTest, RollingValues) {
  EMA ema(5, 2.0);
  ema.update(10.0);
  auto r1 = ema.update(12.0);
  auto r2 = ema.update(14.0);
  ASSERT_TRUE(r1.has_value());
  ASSERT_TRUE(r2.has_value());
  double alpha = 2.0 / (1.0 + 5.0);
  double expected2 = 14.0 * alpha + r1->signal1 * (1.0 - alpha);
  EXPECT_DOUBLE_EQ(r2->signal1, expected2);
}

TEST(EMAComputationTest, Signal2IsCurrentValue) {
  EMA ema(5);
  ema.update(10.0);
  const auto result = ema.update(25.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal2, 25.0);
}

TEST(EMAComputationTest, LargeValues) {
  EMA ema(5);
  ema.update(1e12);
  const auto result = ema.update(2e12);
  ASSERT_TRUE(result.has_value());
  EXPECT_GT(result->signal1, 0.0);
}

TEST(EMAComputationTest, SmallValues) {
  EMA ema(5);
  ema.update(1e-12);
  const auto result = ema.update(2e-12);
  ASSERT_TRUE(result.has_value());
  EXPECT_GT(result->signal1, 0.0);
}

TEST(EMAComputationTest, WithHighSmoothingFactor) {
  EMA ema(5, 5.0);
  ema.update(10.0);
  const auto result = ema.update(20.0);
  ASSERT_TRUE(result.has_value());
  double alpha = 5.0 / (1.0 + 5.0);
  double expected = 20.0 * alpha + 10.0 * (1.0 - alpha);
  EXPECT_DOUBLE_EQ(result->signal1, expected);
}

TEST(EMAComputationTest, WithLowSmoothingFactor) {
  EMA ema(14, 0.5);
  ema.update(10.0);
  const auto result = ema.update(20.0);
  ASSERT_TRUE(result.has_value());
  double alpha = 0.5 / (1.0 + 14.0);
  double expected = 20.0 * alpha + 10.0 * (1.0 - alpha);
  EXPECT_DOUBLE_EQ(result->signal1, expected);
}

// =============================================================================
// Candle integration tests
// =============================================================================

TEST(EMACandleTest, ExtractsCloseFromCandle) {
  EMA ema(5);
  ema.update(make_candle(10.0));
  const auto result = ema.update(make_candle(20.0));
  ASSERT_TRUE(result.has_value());
}

TEST(EMACandleTest, SetsTimestampCorrectly) {
  EMA ema(5);
  ema.update(make_candle(10.0));
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 105.0, 99.0, 103.5, ts};
  const auto result = ema.update(c);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->timestamp, ts);
}

TEST(EMACandleTest, SetsIndicatorId) {
  EMA ema(5);
  ema.update(make_candle(10.0));
  const auto result = ema.update(make_candle(20.0));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, ema.indicator_id());
}

// =============================================================================
// Edge case / boundary tests
// =============================================================================

TEST(EMAEdgeTest, ZeroValues) {
  EMA ema(5);
  ema.update(0.0);
  const auto result = ema.update(0.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 0.0);
}

TEST(EMAEdgeTest, NegativeValues) {
  EMA ema(5);
  ema.update(-10.0);
  const auto result = ema.update(-20.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_LT(result->signal1, -10.0);
}

TEST(EMAEdgeTest, ConvergesOverTime) {
  EMA ema(3, 2.0);
  ema.update(50.0);
  for (int i = 0; i < 100; ++i) {
    ema.update(50.0);
  }
  const auto result = ema.update(50.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(result->signal1, 50.0, 1e-6);
}
