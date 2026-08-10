#include "Indicators/RSI.hpp"

#include <cmath>

namespace TradingEngine::Indicators {
RSI::RSI(std::size_t period) : period_{period}, buffer_{period + 1} {
  if (period == 0) {
    throw std::invalid_argument("Period must not equal to 0");
  }
  id_ = 5; // TODO : place holder fo now
}

std::optional<IndicatorResult> RSI::update(const MarketData::Candle &candle) {
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

std::optional<SignalOutput> RSI::update(double value) noexcept {
  buffer_.push_back(value);
  if (!buffer_.full()) {
    return std::nullopt;
  }
  double up_changes{};
  double down_changes{};

  for (size_t i = 0; i < period_; i++) {
    const double result{buffer_[i + 1] - buffer_[i]};
    if (result > 0) {
      up_changes += result;
    } else {
      down_changes += std::abs(result);
    }
  }
  const double average_up{up_changes / static_cast<double>(period_)};
  const double average_down{down_changes / static_cast<double>(period_)};
  if (average_down == 0.0 && average_up == 0.0) {
    return SignalOutput{
        .signal1{50.0},
        .signal2{threshold_},
    };
  } else if (average_down == 0.0) {
    return SignalOutput{
        .signal1{100.0},
        .signal2{threshold_},
    };
  } else if (average_up == 0.0) {
    return SignalOutput{
        .signal1{0.0},
        .signal2{threshold_},
    };
  }
  const double relative_strength{average_up / average_down};
  const double rsi{100.0 - 100.0 / (1 + relative_strength)};

  return SignalOutput{
      .signal1{rsi},
      .signal2{threshold_},
  };
}
} // namespace TradingEngine::Indicators