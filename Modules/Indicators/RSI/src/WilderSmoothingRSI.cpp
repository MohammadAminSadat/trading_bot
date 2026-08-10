#include "Indicators/WilderSmoothingRSI.hpp"

#include <cmath>

namespace TradingEngine::Indicators {
WilderSmoothingRSI::WilderSmoothingRSI(std::size_t period) : period_{period} {
  if (period == 0) {
    throw std::invalid_argument("Period must not equal to 0");
  }
  id_ = 7; // TODO : place holder fo now
  alpha_ = 1.0 / (static_cast<double>(period));
}

std::optional<IndicatorResult> WilderSmoothingRSI::update(const MarketData::Candle &candle) {
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

std::optional<SignalOutput> WilderSmoothingRSI::update(double value) noexcept {
  if (!previous_value_) {
    previous_value_ = value;
    return std::nullopt;
  }

  const double result{value - previous_value_};
  previous_value_ = value;
  double up_change{};
  double down_change{};
  if (result > 0.0) {
    up_change = result;
  } else {
    down_change = std::abs(result);
  }

  if (warm_up_period_ < period_) {
    previous_up_changes_ += up_change;
    previous_down_changes_ += down_change;
    ++warm_up_period_;

    if (warm_up_period_ == period_) {
      previous_up_changes_ /= static_cast<double>(period_);
      previous_down_changes_ /= static_cast<double>(period_);
    }

    return std::nullopt;
  }

  const double average_up{alpha_ * up_change + (1.0 - alpha_) * previous_up_changes_};
  const double average_down{alpha_ * down_change + (1.0 - alpha_) * previous_down_changes_};
  previous_up_changes_ = average_up;
  previous_down_changes_ = average_down;

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