#pragma once
#include <Indicators/Indicators.hpp>
#include <boost/circular_buffer.hpp>

namespace TradingEngine::Indicators {
class ATR : public IIndicator {
  ATR(std::size_t);
  virtual ~ATR() override = default;
  virtual std::optional<IndicatorResult> update(const MarketData::Candle &) override;
  virtual std::optional<SignalOutput> update(double, double, double) noexcept;
  [[nodiscard]] virtual const InputRequirements &requirements() const override {
    return requirements_;
  };
  [[nodiscard]] virtual SignalType signal_type() const noexcept override {
    return SignalType::ZeroCross;
  };
  IndicatorId indicator_id() { return id_; }

private:
  const std::size_t period_{14};
  double previous_close_{};
  bool warm_up_{true};
  double sum_{};
  boost::circular_buffer<double> buffer_;
  const InputRequirements requirements_{{InputData::Close, InputData::High, InputData::Low}};
  IndicatorId id_{};
};

} // namespace TradingEngine::Indicators