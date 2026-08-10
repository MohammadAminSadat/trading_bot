#include "Indicators/MACD.hpp"

namespace TradingEngine::Indicators {
MACD::MACD(std::size_t fast_period, std::size_t slow_period, std::size_t signal_period)
    : fast_period_{fast_period},
      slow_period_{slow_period},
      signal_period_{signal_period},
      fast_ema_{fast_period},
      slow_ema_{slow_period},
      signal_ema_{signal_period} {
  if (fast_period >= slow_period) {
    throw std::logic_error("fast period must be less than slow period");
  }
  id_ = 4; // TODO: place holder
}

std::optional<IndicatorResult> MACD::update(const MarketData::Candle &candle) {
  const auto output{update(candle.close)};
  if (output) {
    return IndicatorResult{
        .id{id_},
        .timestamp{candle.timestamp},
        .output{*output},
    };
  }
  return std::nullopt;
}

std::optional<SignalOutput> MACD::update(double value) noexcept {
  const auto slow_value{slow_ema_.update(value)};
  const auto fast_value{fast_ema_.update(value)};
  if (!slow_value || !fast_value) {
    return std::nullopt;
  }

  const auto macd_value{fast_value->signal1 - slow_value->signal1};
  const auto signal{signal_ema_.update(macd_value)};
  if (!signal) {
    return std::nullopt;
  }

  return SignalOutput{
      .signal1{macd_value},
      .signal2{signal->signal1},
  };
}
} // namespace TradingEngine::Indicators