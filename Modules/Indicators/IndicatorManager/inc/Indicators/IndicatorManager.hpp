#pragma once
#include <Indicators/Indicators.hpp>
#include <MarketData/candle.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace TradingEngine::Indicators {
class IndicatorManager {
public:
  IndicatorManager() = default;
  ~IndicatorManager() = default;
  std::vector<std::optional<IndicatorResult>> update_all(const MarketData::Candle &);
  void add_indicator(std::unique_ptr<IIndicator> indicator);
  [[nodiscard]]
  const std::vector<InputRequirements> &requirements() const noexcept {
    return requirements_;
  };

private:
  std::vector<std::unique_ptr<IIndicator>> indicators_;
  std::vector<InputRequirements> requirements_;
};
} // namespace TradingEngine::Indicators