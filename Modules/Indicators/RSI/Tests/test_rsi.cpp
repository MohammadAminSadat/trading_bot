#include <Core/Core.hpp>
#include <Indicators/ExponentialRSI.hpp>
#include <Indicators/RSI.hpp>
#include <Indicators/WilderSmoothingRSI.hpp>
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
// RSI - Constructor tests
// =============================================================================

TEST(RSIConstructorTest, DefaultPeriodCreatesValidIndicator) {
  EXPECT_NO_THROW(RSI rsi(14));
}

TEST(RSIConstructorTest, CustomPeriodCreatesValidIndicator) {
  EXPECT_NO_THROW(RSI rsi(5));
  EXPECT_NO_THROW(RSI rsi(20));
}

TEST(RSIConstructorTest, PeriodOneCreatesValidIndicator) {
  EXPECT_NO_THROW(RSI rsi(1));
}

TEST(RSIConstructorTest, ZeroPeriodThrowsInvalidArgument) {
  EXPECT_THROW(RSI rsi(0), std::invalid_argument);
}

TEST(RSIConstructorTest, ZeroPeriodDeathTest) {
  EXPECT_DEATH(
      {
        try {
          RSI(0);
        } catch (const std::exception &) {
          std::abort();
        }
      },
      "");
}

// =============================================================================
// RSI - Signal type and requirements tests
// =============================================================================

TEST(RSISignalTypeTest, ReturnsThresholdCross) {
  RSI rsi(14);
  EXPECT_EQ(rsi.signal_type(), SignalType::ThresholdCross);
}

TEST(RSIRequirementsTest, RequiresCloseOnly) {
  RSI rsi(14);
  const auto &req = rsi.requirements();
  ASSERT_EQ(req.size(), 1);
  EXPECT_EQ(req[0], InputData::Close);
}

// =============================================================================
// RSI - Warmup period tests
// =============================================================================

TEST(RSIWarmupTest, ReturnsNulloptDuringWarmup) {
  RSI rsi(14);
  for (int i = 0; i < 14; ++i) {
    EXPECT_FALSE(rsi.update(50.0 + i));
  }
}

TEST(RSIWarmupTest, ReturnsValueWhenWarmupComplete) {
  RSI rsi(14);
  for (int i = 0; i < 14; ++i) {
    rsi.update(50.0 + i);
  }
  const auto result = rsi.update(64.0);
  ASSERT_TRUE(result.has_value());
}

TEST(RSIWarmupTest, CandleWarmupReturnsNullopt) {
  RSI rsi(14);
  for (int i = 0; i < 14; ++i) {
    EXPECT_FALSE(rsi.update(make_candle(50.0 + i)));
  }
}

TEST(RSIWarmupTest, CandleReturnsValueWhenComplete) {
  RSI rsi(14);
  for (int i = 0; i < 14; ++i) {
    rsi.update(make_candle(50.0 + i));
  }
  const auto result = rsi.update(make_candle(64.0));
  ASSERT_TRUE(result.has_value());
}

// =============================================================================
// RSI - Computation tests
// =============================================================================

TEST(RSIComputationTest, UptrendProducedRSIAbove50) {
  RSI rsi(14);
  for (int i = 0; i < 14; ++i) {
    rsi.update(100.0 + i);
  }
  const auto result = rsi.update(114.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_GT(result->signal1, 80.0);
}

TEST(RSIComputationTest, DowntrendProducedRSIBelow50) {
  RSI rsi(14);
  int base = 200;
  for (int i = 0; i < 14; ++i) {
    rsi.update(static_cast<double>(base - i));
  }
  const auto result = rsi.update(static_cast<double>(base - 14));
  ASSERT_TRUE(result.has_value());
  EXPECT_LT(result->signal1, 20.0);
}

TEST(RSIComputationTest, FlatValuesReturn50) {
  RSI rsi(14);
  for (int i = 0; i < 14; ++i) {
    rsi.update(100.0);
  }
  const auto result = rsi.update(100.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 50.0);
  EXPECT_DOUBLE_EQ(result->signal2, 50.0);
}

TEST(RSIComputationTest, Signal2IsThreshold50) {
  RSI rsi(3);
  rsi.update(10.0);
  rsi.update(12.0);
  rsi.update(14.0);
  const auto result = rsi.update(16.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal2, 50.0);
}

TEST(RSIComputationTest, NoDownChangesReturns100) {
  RSI rsi(2);
  rsi.update(10.0);
  rsi.update(12.0);
  const auto result = rsi.update(14.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 100.0);
}

TEST(RSIComputationTest, NoUpChangesReturns0) {
  RSI rsi(2);
  rsi.update(14.0);
  rsi.update(12.0);
  const auto result = rsi.update(10.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 0.0);
}

TEST(RSIComputationTest, MixedUpDownReturnsCorrectRSI) {
  RSI rsi(3);
  rsi.update(10.0);
  rsi.update(11.0);
  rsi.update(9.0);
  const auto result = rsi.update(13.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(result->signal1, 71.428571428571431, 1e-9);
}

// =============================================================================
// RSI - Candle integration tests
// =============================================================================

TEST(RSICandleTest, SetsTimestampCorrectly) {
  RSI rsi(3);
  rsi.update(make_candle(10.0));
  rsi.update(make_candle(12.0));
  rsi.update(make_candle(14.0));
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 105.0, 99.0, 16.0, ts};
  const auto result = rsi.update(c);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->timestamp, ts);
}

TEST(RSICandleTest, SetsIndicatorId) {
  RSI rsi(3);
  rsi.update(make_candle(10.0));
  rsi.update(make_candle(12.0));
  rsi.update(make_candle(14.0));
  const auto result = rsi.update(make_candle(16.0));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, rsi.indicator_id());
}

// =============================================================================
// RSI - Edge case / boundary tests
// =============================================================================

TEST(RSIEdgeTest, ZeroValues) {
  RSI rsi(3);
  rsi.update(0.0);
  rsi.update(0.0);
  rsi.update(0.0);
  const auto result = rsi.update(0.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 50.0);
}

TEST(RSIEdgeTest, LargeValues) {
  RSI rsi(3);
  rsi.update(1e6);
  rsi.update(1.1e6);
  rsi.update(1.2e6);
  const auto result = rsi.update(1.3e6);
  ASSERT_TRUE(result.has_value());
  EXPECT_GE(result->signal1, 0.0);
  EXPECT_LE(result->signal1, 100.0);
}

// =============================================================================
// ExponentialRSI - Constructor tests
// =============================================================================

TEST(ExponentialRSIConstructorTest, DefaultPeriodCreatesValidIndicator) {
  EXPECT_NO_THROW(ExponentialRSI rsi(14));
}

TEST(ExponentialRSIConstructorTest, ZeroPeriodThrowsInvalidArgument) {
  EXPECT_THROW(ExponentialRSI rsi(0), std::invalid_argument);
}

TEST(ExponentialRSIConstructorTest, ZeroPeriodDeathTest) {
  EXPECT_DEATH(
      {
        try {
          ExponentialRSI(0);
        } catch (const std::exception &) {
          std::abort();
        }
      },
      "");
}

// =============================================================================
// ExponentialRSI - Signal type and requirements tests
// =============================================================================

TEST(ExponentialRSISignalTypeTest, ReturnsThresholdCross) {
  ExponentialRSI rsi(14);
  EXPECT_EQ(rsi.signal_type(), SignalType::ThresholdCross);
}

TEST(ExponentialRSIRequirementsTest, RequiresCloseOnly) {
  ExponentialRSI rsi(14);
  const auto &req = rsi.requirements();
  ASSERT_EQ(req.size(), 1);
  EXPECT_EQ(req[0], InputData::Close);
}

// =============================================================================
// ExponentialRSI - Warmup period tests
// =============================================================================

TEST(ExponentialRSIWarmupTest, ReturnsNulloptDuringWarmup) {
  ExponentialRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    EXPECT_FALSE(rsi.update(50.0 + i));
  }
}

TEST(ExponentialRSIWarmupTest, ReturnsValueAfterWarmup) {
  ExponentialRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(50.0 + i);
  }
  const auto result = rsi.update(64.0);
  ASSERT_TRUE(result.has_value());
}

// =============================================================================
// ExponentialRSI - Computation tests
// =============================================================================

TEST(ExponentialRSIComputationTest, UptrendProducesHighRSI) {
  ExponentialRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(100.0 + i);
  }
  const auto result = rsi.update(114.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_GT(result->signal1, 80.0);
}

TEST(ExponentialRSIComputationTest, DowntrendProducesLowRSI) {
  ExponentialRSI rsi(14);
  int base = 200;
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(static_cast<double>(base - i));
  }
  const auto result = rsi.update(static_cast<double>(base - 15));
  ASSERT_TRUE(result.has_value());
  EXPECT_LT(result->signal1, 20.0);
}

TEST(ExponentialRSIComputationTest, FlatValuesReturn50) {
  ExponentialRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(100.0);
  }
  const auto result = rsi.update(100.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(result->signal1, 50.0, 1e-10);
  EXPECT_DOUBLE_EQ(result->signal2, 50.0);
}

TEST(ExponentialRSIComputationTest, Signal2IsThreshold50) {
  ExponentialRSI rsi(3);
  for (int i = 0; i < 3 + 1; ++i) {
    rsi.update(10.0 + i);
  }
  const auto result = rsi.update(14.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal2, 50.0);
}

TEST(ExponentialRSIComputationTest, NoDownChangesReturns100) {
  ExponentialRSI rsi(2);
  for (int i = 0; i < 2 + 1; ++i) {
    rsi.update(10.0 + i);
  }
  const auto result = rsi.update(13.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 100.0);
}

TEST(ExponentialRSIComputationTest, RSIInRange0To100) {
  ExponentialRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(50.0 + std::sin(i * 0.5) * 20.0);
  }
  for (int i = 0; i < 50; ++i) {
    const auto result = rsi.update(50.0 + std::sin((15 + i) * 0.5) * 20.0);
    ASSERT_TRUE(result.has_value());
    EXPECT_GE(result->signal1, 0.0);
    EXPECT_LE(result->signal1, 100.0);
  }
}

// =============================================================================
// ExponentialRSI - Candle integration tests
// =============================================================================

TEST(ExponentialRSICandleTest, SetsTimestampCorrectly) {
  ExponentialRSI rsi(3);
  for (int i = 0; i < 3 + 1; ++i) {
    rsi.update(make_candle(10.0 + i));
  }
  auto ts = make_ts(2024, 6, 15);
  Candle c{100.0, 105.0, 99.0, 14.0, ts};
  const auto result = rsi.update(c);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->timestamp, ts);
}

// =============================================================================
// WilderSmoothingRSI - Constructor tests
// =============================================================================

TEST(WilderSmoothingRSIConstructorTest, DefaultPeriodCreatesValidIndicator) {
  EXPECT_NO_THROW(WilderSmoothingRSI rsi(14));
}

TEST(WilderSmoothingRSIConstructorTest, ZeroPeriodThrowsInvalidArgument) {
  EXPECT_THROW(WilderSmoothingRSI rsi(0), std::invalid_argument);
}

TEST(WilderSmoothingRSIConstructorTest, ZeroPeriodDeathTest) {
  EXPECT_DEATH(
      {
        try {
          WilderSmoothingRSI(0);
        } catch (const std::exception &) {
          std::abort();
        }
      },
      "");
}

// =============================================================================
// WilderSmoothingRSI - Signal type and requirements tests
// =============================================================================

TEST(WilderSmoothingRSISignalTypeTest, ReturnsThresholdCross) {
  WilderSmoothingRSI rsi(14);
  EXPECT_EQ(rsi.signal_type(), SignalType::ThresholdCross);
}

TEST(WilderSmoothingRSIRequirementsTest, RequiresCloseOnly) {
  WilderSmoothingRSI rsi(14);
  const auto &req = rsi.requirements();
  ASSERT_EQ(req.size(), 1);
  EXPECT_EQ(req[0], InputData::Close);
}

// =============================================================================
// WilderSmoothingRSI - Warmup period tests
// =============================================================================

TEST(WilderSmoothingRSIWarmupTest, ReturnsNulloptDuringWarmup) {
  WilderSmoothingRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    EXPECT_FALSE(rsi.update(50.0 + i));
  }
}

TEST(WilderSmoothingRSIWarmupTest, ReturnsValueAfterWarmup) {
  WilderSmoothingRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(50.0 + i);
  }
  const auto result = rsi.update(64.0);
  ASSERT_TRUE(result.has_value());
}

// =============================================================================
// WilderSmoothingRSI - Computation tests
// =============================================================================

TEST(WilderSmoothingRSIComputationTest, UptrendProducesHighRSI) {
  WilderSmoothingRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(100.0 + i);
  }
  const auto result = rsi.update(114.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_GT(result->signal1, 80.0);
}

TEST(WilderSmoothingRSIComputationTest, DowntrendProducesLowRSI) {
  WilderSmoothingRSI rsi(14);
  int base = 200;
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(static_cast<double>(base - i));
  }
  const auto result = rsi.update(static_cast<double>(base - 15));
  ASSERT_TRUE(result.has_value());
  EXPECT_LT(result->signal1, 20.0);
}

TEST(WilderSmoothingRSIComputationTest, FlatValuesReturn50) {
  WilderSmoothingRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(100.0);
  }
  const auto result = rsi.update(100.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_NEAR(result->signal1, 50.0, 1e-10);
  EXPECT_DOUBLE_EQ(result->signal2, 50.0);
}

TEST(WilderSmoothingRSIComputationTest, Signal2IsThreshold50) {
  WilderSmoothingRSI rsi(3);
  for (int i = 0; i < 3 + 1; ++i) {
    rsi.update(10.0 + i);
  }
  const auto result = rsi.update(14.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal2, 50.0);
}

TEST(WilderSmoothingRSIComputationTest, RSIInRange0To100) {
  WilderSmoothingRSI rsi(14);
  for (int i = 0; i < 14 + 1; ++i) {
    rsi.update(50.0 + std::sin(i * 0.5) * 20.0);
  }
  for (int i = 0; i < 50; ++i) {
    const auto result = rsi.update(50.0 + std::sin((15 + i) * 0.5) * 20.0);
    ASSERT_TRUE(result.has_value());
    EXPECT_GE(result->signal1, 0.0);
    EXPECT_LE(result->signal1, 100.0);
  }
}

TEST(WilderSmoothingRSIComputationTest, WilderConvergesDifferentlyFromExponential) {
  ExponentialRSI exp_rsi(14);
  WilderSmoothingRSI wilder_rsi(14);
  const double values[] = {44.34, 44.09, 44.15, 43.61, 44.33, 44.83, 45.10, 45.42, 45.84,
                           46.08, 45.89, 46.03, 45.61, 46.28, 46.28, 46.00, 46.03};
  for (double v : values) {
    exp_rsi.update(v);
    wilder_rsi.update(v);
  }
  const auto exp_result = exp_rsi.update(46.41);
  const auto wilder_result = wilder_rsi.update(46.41);
  ASSERT_TRUE(exp_result.has_value());
  ASSERT_TRUE(wilder_result.has_value());
  EXPECT_FALSE(std::isnan(exp_result->signal1));
  EXPECT_FALSE(std::isnan(wilder_result->signal1));
}

// =============================================================================
// RSI edge case: consistent monotonic price series
// =============================================================================

TEST(RSIEdgeTest, MonotonicUpwardAll100) {
  RSI rsi(5);
  for (int i = 0; i < 5; ++i) {
    rsi.update(static_cast<double>(10 + i));
  }
  const auto result = rsi.update(15.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 100.0);
}

TEST(RSIEdgeTest, MonotonicDownwardAll0) {
  RSI rsi(5);
  for (int i = 0; i < 5; ++i) {
    rsi.update(static_cast<double>(20 - i));
  }
  const auto result = rsi.update(15.0);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->signal1, 0.0);
}
