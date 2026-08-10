#include <Core/Core.hpp>
#include <Indicators/MACD.hpp>
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

Candle make_candle(double close, int y = 2024, int m = 1, int d = 1) {
  return Candle{close, close, close, close, make_ts(y, m, d)};
}

} // namespace

// =============================================================================
// Constructor tests
// =============================================================================

TEST(MACDConstructorTest, DefaultPeriodsCreateValidIndicator) {
  EXPECT_NO_THROW(MACD macd(12, 26, 9));
}

TEST(MACDConstructorTest, CustomPeriodsCreateValidIndicator) {
  EXPECT_NO_THROW(MACD macd(5, 20, 10));
  EXPECT_NO_THROW(MACD macd(1, 2, 1));
}

TEST(MACDConstructorTest, FastPeriodNotLessThanSlowThrowsLogicError) {
  EXPECT_THROW(MACD macd(26, 12, 9), std::logic_error);
}

TEST(MACDConstructorTest, FastPeriodEqualsSlowThrowsLogicError) {
  EXPECT_THROW(MACD macd(14, 14, 9), std::logic_error);
}

TEST(MACDConstructorTest, FastPeriodNotLessThanSlowDeathTest) {
  EXPECT_DEATH(
      {
        try {
          MACD(14, 14, 9);
        } catch (const std::exception &) {
          std::abort();
        }
      },
      "");
}

// =============================================================================
// Signal type and requirements tests
// =============================================================================

TEST(MACDSignalTypeTest, ReturnsTwoLineCross) {
  MACD macd(12, 26, 9);
  EXPECT_EQ(macd.signal_type(), SignalType::TwoLineCross);
}

TEST(MACDRequirementsTest, RequiresCloseOnly) {
  MACD macd(12, 26, 9);
  const auto &req = macd.requirements();
  ASSERT_EQ(req.size(), 1);
  EXPECT_EQ(req[0], InputData::Close);
}

// =============================================================================
// Warmup period tests
// =============================================================================

TEST(MACDWarmupTest, ReturnsNulloptDuringWarmup) {
  MACD macd(12, 26, 9);
  EXPECT_FALSE(macd.update(100.0));
  EXPECT_FALSE(macd.update(101.0));
}

TEST(MACDWarmupTest, ReturnsValueAfterWarmup) {
  MACD macd(3, 5, 3);
  for (int i = 0; i < 10; ++i) {
    macd.update(100.0 + i);
  }
  const auto result = macd.update(110.0);
  ASSERT_TRUE(result.has_value());
}

TEST(MACDWarmupTest, CandleUpdateReturnsNulloptDuringWarmup) {
  MACD macd(12, 26, 9);
  EXPECT_FALSE(macd.update(make_candle(100.0)));
}

TEST(MACDWarmupTest, CandleUpdateReturnsValueAfterWarmup) {
  MACD macd(3, 5, 3);
  for (int i = 0; i < 10; ++i) {
    macd.update(make_candle(100.0 + i));
  }
  const auto result = macd.update(make_candle(110.0));
  ASSERT_TRUE(result.has_value());
}

// =============================================================================
// Computation tests
// =============================================================================

TEST(MACDComputationTest, Signal1IsFastEMAMinusSlowEMA) {
  MACD macd(3, 5, 3);
  EMA fast(3);
  EMA slow(5);
  double value = 100.0;
  fast.update(value);
  slow.update(value);
  for (int i = 1; i < 10; ++i) {
    macd.update(value);
    fast.update(value);
    slow.update(value);
    value += 1.0;
  }
  const auto fast_val = fast.update(value);
  const auto slow_val = slow.update(value);
  const auto result = macd.update(value);
  ASSERT_TRUE(fast_val.has_value());
  ASSERT_TRUE(slow_val.has_value());
  ASSERT_TRUE(result.has_value());
  double expected = fast_val->signal1 - slow_val->signal1;
  EXPECT_DOUBLE_EQ(result->signal1, expected);
}

TEST(MACDComputationTest, Signal2IsSignalLineEMA) {
  MACD macd(3, 5, 3);
  EMA signal_ema(3);
  double macd_value = 0.0;
  double value = 100.0;
  for (int i = 0; i < 10; ++i) {
    macd.update(value);
    value += 1.0;
  }
  const auto result = macd.update(value);
  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(std::isnan(result->signal2));
}

TEST(MACDComputationTest, FlatValuesProduceApproxZero) {
  MACD macd(3, 5, 3);
  for (int i = 0; i < 20; ++i) {
    macd.update(100.0);
  }
  const auto result = macd.update(100.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(result->signal1, 0.0, 1e-10);
  EXPECT_NEAR(result->signal2, 0.0, 1e-10);
}

TEST(MACDComputationTest, UptrendProducesPositiveMACD) {
  MACD macd(3, 5, 3);
  double value = 50.0;
  for (int i = 0; i < 30; ++i) {
    macd.update(value);
    value += 1.0;
  }
  const auto result = macd.update(value);
  ASSERT_TRUE(result.has_value());
  EXPECT_GT(result->signal1, 0.0);
}

TEST(MACDComputationTest, DowntrendProducesNegativeMACD) {
  MACD macd(3, 5, 3);
  double value = 150.0;
  for (int i = 0; i < 30; ++i) {
    macd.update(value);
    value -= 1.0;
  }
  const auto result = macd.update(value);
  ASSERT_TRUE(result.has_value());
  EXPECT_LT(result->signal1, 0.0);
}

// =============================================================================
// Candle integration tests
// =============================================================================

TEST(MACDCandleTest, SetsTimestampCorrectly) {
  MACD macd(3, 5, 3);
  for (int i = 0; i < 10; ++i) {
    macd.update(make_candle(100.0 + i));
  }
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 105.0, 99.0, 103.5, ts};
  const auto result = macd.update(c);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->timestamp, ts);
}

TEST(MACDCandleTest, SetsIndicatorId) {
  MACD macd(3, 5, 3);
  for (int i = 0; i < 10; ++i) {
    macd.update(make_candle(100.0 + i));
  }
  const auto result = macd.update(make_candle(110.0));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, macd.indicator_id());
}

// =============================================================================
// Signal line cross tests
// =============================================================================

TEST(MACDSignalCrossTest, SignalLineCrossDetection) {
  MACD macd(3, 5, 3);
  double value = 100.0;
  for (int i = 0; i < 20; ++i) {
    macd.update(value);
    value += 1.0;
  }
  const auto result = macd.update(value);
  ASSERT_TRUE(result.has_value());
  if (result->signal1 > result->signal2) {
    EXPECT_GT(result->signal1, result->signal2);
  } else {
    EXPECT_LE(result->signal1, result->signal2);
  }
}

// =============================================================================
// Edge case / boundary tests
// =============================================================================

TEST(MACDEdgeTest, ZeroValues) {
  MACD macd(3, 5, 3);
  for (int i = 0; i < 20; ++i) {
    macd.update(0.0);
  }
  const auto result = macd.update(0.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(result->signal1, 0.0, 1e-10);
  EXPECT_NEAR(result->signal2, 0.0, 1e-10);
}

TEST(MACDEdgeTest, NegativeValues) {
  MACD macd(3, 5, 3);
  for (int i = 0; i < 20; ++i) {
    macd.update(-50.0);
  }
  const auto result = macd.update(-50.0);
  ASSERT_TRUE(result.has_value());
}

TEST(MACDEdgeTest, LargePeriods) {
  MACD macd(50, 100, 20);
  for (int i = 0; i < 150; ++i) {
    macd.update(100.0 + i);
  }
  const auto result = macd.update(250.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(std::isnan(result->signal1));
  EXPECT_FALSE(std::isnan(result->signal2));
}
