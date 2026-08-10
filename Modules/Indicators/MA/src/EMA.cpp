#include "Indicators/EMA.hpp"

namespace TradingEngine::Indicators {
EMA::EMA(std::size_t period, double smoothing_factor)
    : period_{period},
      smoothing_factor_{smoothing_factor_},
      alpha_{smoothing_factor_ / (1 + static_cast<double>(period))} {
  if (period == 0) {
    throw std::invalid_argument("Period must not equal to 0");
  }
  id_ = 2; // TODO : place holder fo now
}

std::optional<SignalOutput> EMA::update(double value) noexcept {
  if (warm_up_) {
    previous_ema_ = value;
    warm_up_ = false;
    return std::nullopt;
  }

  previous_ema_ = value * alpha_ + (1 - alpha_) * previous_ema_;
  return SignalOutput{previous_ema_, value};
}

std::optional<IndicatorResult> EMA::update(const MarketData::Candle &candle) {
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

} // namespace TradingEngine::Indicators
