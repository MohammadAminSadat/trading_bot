#include "Indicators/SMA.hpp"

#include <numeric>

namespace TradingEngine::Indicators {
SMA::SMA(std::size_t period) : period{period}, cache{period} {
  if (period == 0) {
    throw std::invalid_argument("Period must not equal to 0");
  }
  id = 1; // must be calculated by a hash
}

std::optional<SignalOutput> SMA::update(double value) noexcept {
  if (cache.full()) {
    sum_ -= cache.front();
  }

  cache.push_back(value);
  sum_ += value;

  if (!cache.full()) {
    return std::nullopt;
  }

  const double output = sum_ / static_cast<double>(period);

  return SignalOutput{
      .signal1 = output,
      .signal2 = value,
  };
}

std::optional<IndicatorResult> SMA::update(const MarketData::Candle &candle) {
  const auto output{update(candle.close)};
  if (output) {
    return IndicatorResult{
        .id{id},
        .timestamp{candle.timestamp},
        .output{*output},
    };
  }

  return std::nullopt;
}

} // namespace TradingEngine::Indicators
