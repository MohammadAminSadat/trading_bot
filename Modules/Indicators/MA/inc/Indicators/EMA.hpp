#pragma once
#include <Indicators/Indicators.hpp>

namespace TradingEngine::Indicators {
class EMA : public IIndicator {
  EMA(std::size_t, double);
  explicit EMA(std::size_t);
  virtual ~EMA() override = default;
  virtual std::optional<IndicatorResult> update(const MarketData::Candle &) override;
  virtual std::optional<SignalOutput> update(double) noexcept;
  [[nodiscard]] virtual const InputRequirements &requirements() const override {
    return requirements_;
  };
  [[nodiscard]] virtual SignalType signal_type() const noexcept override {
    return SignalType::PriceCross;
  };
  IndicatorId indicator_id() { return id_; }

private:
  const std::size_t period_{14};
  double smoothing_factor_{2.0};
  double alpha_{};
  double previous_ema_{};
  const InputRequirements requirements_{{InputData::Close}};
  IndicatorId id_{};
  bool warm_up_{true};
};

} // namespace TradingEngine::Indicators