#include "Indicators/IndicatorManager.hpp"

namespace TradingEngine::Indicators {
std::vector<std::optional<IndicatorResult>> IndicatorManager::update_all(
    const MarketData::Candle &candle) {
  std::vector<std::optional<IndicatorResult>> output;
  output.reserve(indicators_.size());

  for (const auto &indicator : indicators_) {
    output.push_back(indicator->update(candle));
  }
  return output;
}

void IndicatorManager::add_indicator(std::unique_ptr<IIndicator> indicator) {
  requirements_.push_back(indicator->requirements());
  indicators_.push_back(std::move(indicator));
}

} // namespace TradingEngine::Indicators
