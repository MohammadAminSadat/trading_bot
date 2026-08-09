#include "Indicators/IndicatorManager.hpp"

namespace TradingEngine::Indicators {
std::vector<IndicatorOutput> IndicatorManager::update_all(const MarketData::Candle &candle) {
  std::vector<IndicatorOutput> output;

  for (const auto &indicator : indicators_) {
    auto indicator_output{indicator->update(candle)};
    if (indicator_output) {
      output.push_back(*indicator_output);
    } else {
      return std::vector<IndicatorOutput>();
    }
  }
  return output;
}

void IndicatorManager::add_indicator(std::unique_ptr<IIndicator> indicator) {
  requirements_.push_back(indicator->requirements());
  indicators_.push_back(std::move(indicator));
}

} // namespace TradingEngine::Indicators
