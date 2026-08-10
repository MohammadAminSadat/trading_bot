#include "Indicators/ATR.hpp"

#include <cmath>

namespace TradingEngine::Indicators {
ATR::ATR(std::size_t period) : period_{period}, buffer_{period} {
  if (period == 0) {
    throw std::invalid_argument("Period must not equal to 0");
  }
  id_ = 3; // TODO: place holder
}

std::optional<IndicatorResult> ATR::update(const MarketData::Candle &candle) {
  const auto output{update(candle.close, candle.high, candle.low)};
  if (output) {
    return IndicatorResult{
        .id{id_},
        .timestamp{candle.timestamp},
        .output{*output},
    };
  }
  return std::nullopt;
}

std::optional<SignalOutput> ATR::update(double close, double high, double low) noexcept {
  if (warm_up_) {
    previous_close_ = close;
    warm_up_ = false;
    return std::nullopt;
  }
  double true_range{std::max({
      std::abs(high - low),
      std::abs(previous_close_ - high),
      std::abs(previous_close_ - low),
  })};

  if (buffer_.full()) {
    sum_ -= buffer_.front();
  }

  sum_ += true_range;
  buffer_.push_back(true_range);

  if (!buffer_.full()) {
    return std::nullopt;
  }

  const double output{sum_ / static_cast<double>(period_)};
  previous_close_ = close;

  return SignalOutput{
      .signal1{output},
      .signal2{0.0},
  };
}

} // namespace TradingEngine::Indicators