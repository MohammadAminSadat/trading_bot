#pragma once
#include <Indicators/Indicators.hpp>

namespace TradingEngine::Indicators {
class ExponentialRSI : public IIndicator {
public:
  explicit ExponentialRSI(std::size_t);
  virtual ~ExponentialRSI() override = default;
  virtual std::optional<IndicatorResult> update(const MarketData::Candle &) override;
  virtual std::optional<SignalOutput> update(double) noexcept;
  [[nodiscard]] virtual const InputRequirements &requirements() const override {
    return requirements_;
  };
  [[nodiscard]] virtual SignalType signal_type() const noexcept override {
    return SignalType::ThresholdCross;
  };
  IndicatorId indicator_id() { return id_; }

private:
  const std::size_t period_{14};
  const InputRequirements requirements_{{InputData::Close}};
  IndicatorId id_{};
  const double threshold_{50};
  double previous_up_changes_{};
  double previous_down_changes_{};
  double previous_value_{};
  double alpha_{};
  std::size_t warm_up_period_{0};
};

} // namespace TradingEngine::Indicators