#include <Core/Core.hpp>
#include <Indicators/SMA.hpp>
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

TEST(SMAConstructorTest, DefaultPeriodCreatesValidIndicator) {
  EXPECT_NO_THROW(SMA sma(14));
}

TEST(SMAConstructorTest, CustomPeriodCreatesValidIndicator) {
  EXPECT_NO_THROW(SMA sma(5));
  EXPECT_NO_THROW(SMA sma(50));
  EXPECT_NO_THROW(SMA sma(200));
}

TEST(SMAConstructorTest, PeriodOneCreatesValidIndicator) {
  EXPECT_NO_THROW(SMA sma(1));
}

TEST(SMAConstructorTest, ZeroPeriodThrowsInvalidArgument) {
  EXPECT_THROW(SMA sma(0), std::invalid_argument);
}

TEST(SMAConstructorTest, ZeroPeriodDeathTest) {
  EXPECT_DEATH(
      {
        try {
          SMA(0);
        } catch (const std::exception &) {
          std::abort();
        }
      },
      "");
}

// =============================================================================
// Signal type and requirements tests
// =============================================================================

TEST(SMASignalTypeTest, ReturnsPriceCross) {
  SMA sma(14);
  EXPECT_EQ(sma.signal_type(), SignalType::PriceCross);
}

TEST(SMARequirementsTest, RequiresCloseOnly) {
  SMA sma(14);
  const auto &req = sma.requirements();
  ASSERT_EQ(req.size(), 1);
  EXPECT_EQ(req[0], InputData::Close);
}

// =============================================================================
// Warmup period tests
// =============================================================================

TEST(SMAWarmupTest, ReturnsNulloptDuringWarmup) {
  SMA sma(3);
  EXPECT_FALSE(sma.update(make_candle(10.0)));
  EXPECT_FALSE(sma.update(make_candle(20.0)));
}

TEST(SMAWarmupTest, ReturnsValueWhenWarmupComplete) {
  SMA sma(3);
  sma.update(make_candle(10.0));
  sma.update(make_candle(20.0));
  const auto result = sma.update(make_candle(30.0));
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->output.signal1, 20.0);
}

TEST(SMAWarmupTest, DoubleUpdateReturnsNulloptDuringWarmup) {
  SMA sma(3);
  EXPECT_FALSE(sma.update(10.0));
  EXPECT_FALSE(sma.update(20.0));
}

TEST(SMAWarmupTest, DoubleUpdateReturnsValueWhenWarmupComplete) {
  SMA sma(3);
  sma.update(10.0);
  sma.update(20.0);
  const auto result = sma.update(30.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 20.0);
}

TEST(SMAWarmupTest, PeriodOneReturnsValueImmediately) {
  SMA sma(1);
  const auto result = sma.update(100.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 100.0);
}

TEST(SMAWarmupTest, PeriodOneCandleReturnsValueImmediately) {
  SMA sma(1);
  const auto result = sma.update(make_candle(100.0));
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->output.signal1, 100.0);
}

// =============================================================================
// Computation tests
// =============================================================================

TEST(SMAComputationTest, ComputesCorrectAverage) {
  SMA sma(5);
  sma.update(10.0);
  sma.update(20.0);
  sma.update(30.0);
  sma.update(40.0);
  const auto result = sma.update(50.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 30.0);
}

TEST(SMAComputationTest, RollingWindowUpdatesCorrectly) {
  SMA sma(3);
  sma.update(10.0);
  sma.update(20.0);
  auto r1 = sma.update(30.0);
  ASSERT_TRUE(r1.has_value());
  EXPECT_DOUBLE_EQ(r1->signal1, 20.0);

  auto r2 = sma.update(40.0);
  ASSERT_TRUE(r2.has_value());
  EXPECT_DOUBLE_EQ(r2->signal1, 30.0);

  auto r3 = sma.update(50.0);
  ASSERT_TRUE(r3.has_value());
  EXPECT_DOUBLE_EQ(r3->signal1, 40.0);
}

TEST(SMAComputationTest, Signal2IsCurrentValue) {
  SMA sma(2);
  sma.update(10.0);
  const auto result = sma.update(30.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 20.0);
  EXPECT_DOUBLE_EQ(result->signal2, 30.0);
}

TEST(SMAComputationTest, LargeValues) {
  SMA sma(2);
  sma.update(1e12);
  const auto result = sma.update(1e12);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 1e12);
}

TEST(SMAComputationTest, SmallValues) {
  SMA sma(2);
  sma.update(1e-12);
  const auto result = sma.update(1e-12);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 1e-12);
}

// =============================================================================
// Candle integration tests
// =============================================================================

TEST(SMACandleTest, ExtractsCloseFromCandle) {
  SMA sma(2);
  sma.update(make_candle(15.0));
  const auto result = sma.update(make_candle(25.0));
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->output.signal1, 20.0);
}

TEST(SMACandleTest, SetsTimestampCorrectly) {
  SMA sma(1);
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 105.0, 99.0, 103.5, ts};
  const auto result = sma.update(c);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->timestamp, ts);
}

TEST(SMACandleTest, SetsIndicatorId) {
  SMA sma(1);
  const auto result = sma.update(make_candle(100.0));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, sma.indicator_id());
}

// =============================================================================
// Edge case / boundary tests
// =============================================================================

TEST(SMAEdgeTest, ZeroValues) {
  SMA sma(3);
  sma.update(0.0);
  sma.update(0.0);
  const auto result = sma.update(0.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 0.0);
}

TEST(SMAEdgeTest, NegativeValues) {
  SMA sma(2);
  sma.update(-10.0);
  const auto result = sma.update(-20.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, -15.0);
}

TEST(SMAEdgeTest, MixedSignValues) {
  SMA sma(3);
  sma.update(-10.0);
  sma.update(0.0);
  const auto result = sma.update(10.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 0.0);
}

TEST(SMAEdgeTest, LargePeriodBufferRotation) {
  SMA sma(5);
  sma.update(100.0);
  sma.update(200.0);
  sma.update(300.0);
  sma.update(400.0);
  auto r1 = sma.update(500.0);
  ASSERT_TRUE(r1.has_value());
  EXPECT_DOUBLE_EQ(r1->signal1, 300.0);

  sma.update(600.0);
  sma.update(700.0);
  sma.update(800.0);
  auto r5 = sma.update(900.0);
  ASSERT_TRUE(r5.has_value());
  EXPECT_DOUBLE_EQ(r5->signal1, 700.0);
}
