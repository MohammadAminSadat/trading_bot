#include <Core/Core.hpp>
#include <Indicators/ATR.hpp>
#include <Indicators/EMA.hpp>
#include <Indicators/IndicatorManager.hpp>
#include <Indicators/SMA.hpp>
#include <chrono>
#include <cmath>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <vector>

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
// Empty manager tests
// =============================================================================

TEST(IndicatorManagerTest, DefaultConstructedIsEmpty) {
  IndicatorManager manager;
  EXPECT_TRUE(manager.requirements().empty());
}

TEST(IndicatorManagerTest, UpdateAllWithNoIndicatorsReturnsEmpty) {
  IndicatorManager manager;
  auto c = make_candle(100.0, 110.0, 90.0, 105.0);
  auto results = manager.update_all(c);
  EXPECT_TRUE(results.empty());
}

// =============================================================================
// Single indicator tests
// =============================================================================

TEST(IndicatorManagerTest, AddSingleIndicator) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(14));
  EXPECT_EQ(manager.requirements().size(), 1);
}

TEST(IndicatorManagerTest, UpdateAllWithSingleIndicatorDuringWarmup) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(14));
  auto c = make_candle(100.0, 110.0, 90.0, 105.0);
  auto results = manager.update_all(c);
  ASSERT_EQ(results.size(), 1);
  EXPECT_FALSE(results[0].has_value());
}

TEST(IndicatorManagerTest, UpdateAllWithSingleIndicatorAfterWarmup) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(3));
  manager.update_all(make_candle(10.0, 10.0, 10.0, 10.0));
  manager.update_all(make_candle(20.0, 20.0, 20.0, 20.0));
  auto results = manager.update_all(make_candle(30.0, 30.0, 30.0, 30.0));
  ASSERT_EQ(results.size(), 1);
  ASSERT_TRUE(results[0].has_value());
  EXPECT_DOUBLE_EQ(results[0]->output.signal1, 20.0);
}

// =============================================================================
// Multiple indicator tests
// =============================================================================

TEST(IndicatorManagerTest, AddMultipleIndicators) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(14));
  manager.add_indicator(std::make_unique<EMA>(14));
  EXPECT_EQ(manager.requirements().size(), 2);
}

TEST(IndicatorManagerTest, RequirementsAggregatedCorrectly) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(14));
  manager.add_indicator(std::make_unique<ATR>(14));

  const auto &reqs = manager.requirements();
  ASSERT_EQ(reqs.size(), 2);
  EXPECT_EQ(reqs[0].size(), 1);
  EXPECT_EQ(reqs[1].size(), 3);
}

TEST(IndicatorManagerTest, UpdateAllReturnsResultsInInsertionOrder) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(3));
  manager.add_indicator(std::make_unique<EMA>(14));

  manager.update_all(make_candle(10.0, 10.0, 10.0, 10.0));
  manager.update_all(make_candle(20.0, 20.0, 20.0, 20.0));
  auto results = manager.update_all(make_candle(30.0, 30.0, 30.0, 30.0));

  ASSERT_EQ(results.size(), 2);
  EXPECT_TRUE(results[0].has_value());
}

TEST(IndicatorManagerTest, MultipleIndicatorsAllProduceResultsAfterWarmup) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(3));
  manager.add_indicator(std::make_unique<EMA>(14));

  for (int i = 0; i < 20; ++i) {
    manager.update_all(make_candle(100.0 + i, 100.0 + i, 100.0 + i, 100.0 + i));
  }
  auto results = manager.update_all(make_candle(120.0, 120.0, 120.0, 120.0));

  ASSERT_EQ(results.size(), 2);
  EXPECT_TRUE(results[0].has_value());
  EXPECT_TRUE(results[1].has_value());
}

TEST(IndicatorManagerTest, ThreeDifferentIndicatorsProduceCorrectCount) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(3));
  manager.add_indicator(std::make_unique<EMA>(14));
  manager.add_indicator(std::make_unique<ATR>(3));

  for (int i = 0; i < 20; ++i) {
    manager.update_all(make_candle(100.0 + i, 110.0 + i, 90.0 + i, 105.0 + i));
  }
  auto results = manager.update_all(make_candle(120.0, 130.0, 100.0, 125.0));
  EXPECT_EQ(results.size(), 3);
}

// =============================================================================
// Move semantics tests
// =============================================================================

TEST(IndicatorManagerTest, IndicatorOwnershipIsTransferred) {
  IndicatorManager manager;
  auto sma_ptr = std::make_unique<SMA>(14);
  auto *raw_ptr = sma_ptr.get();
  manager.add_indicator(std::move(sma_ptr));
  EXPECT_TRUE(static_cast<bool>(raw_ptr));
}

TEST(IndicatorManagerTest, MoveIndicatorsOutIsNotAllowed) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(14));
  auto results = manager.update_all(make_candle(100.0, 110.0, 90.0, 105.0));
  EXPECT_EQ(results.size(), 1);
}

// =============================================================================
// Mixed warmup state tests
// =============================================================================

TEST(IndicatorManagerTest, MixedWarmupStatesReturnMixedResults) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(30));
  manager.add_indicator(std::make_unique<EMA>(14));

  manager.update_all(make_candle(10.0, 10.0, 10.0, 10.0));
  manager.update_all(make_candle(20.0, 20.0, 20.0, 20.0));
  auto results = manager.update_all(make_candle(30.0, 30.0, 30.0, 30.0));

  EXPECT_FALSE(results[0].has_value());
  EXPECT_TRUE(results[1].has_value());
}

// =============================================================================
// Requirements aggregation edge cases
// =============================================================================

TEST(IndicatorManagerTest, MultipleIndicatorsAllClose) {
  IndicatorManager manager;
  manager.add_indicator(std::make_unique<SMA>(14));
  manager.add_indicator(std::make_unique<EMA>(14));

  const auto &reqs = manager.requirements();
  for (const auto &req : reqs) {
    ASSERT_EQ(req.size(), 1);
    EXPECT_EQ(req[0], InputData::Close);
  }
}
