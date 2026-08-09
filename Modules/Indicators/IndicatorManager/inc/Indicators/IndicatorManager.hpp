#pragma once
#include <Indicators/indicators.hpp>
#include <MarketData/candle.hpp>
#include <memory>
#include <vector>

namespace TradingEngine::Indicators {
class IndicatorManager {
public:
  IndicatorManager() = default;
  ~IndicatorManager() = default;
  std::vector<IndicatorOutput> update_all(const MarketData::Candle &);
  void add_indicator(std::unique_ptr<IIndicator> indicator);
  const std::vector<InputRequirements> &requirements() { return requirements_; };

private:
  std::vector<std::unique_ptr<IIndicator>> indicators_;
  std::vector<InputRequirements> requirements_;
};
} // namespace TradingEngine::Indicators