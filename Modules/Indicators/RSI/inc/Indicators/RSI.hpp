#pragma once
#include <Indicators/Indicators.hpp>
#include <boost/circular_buffer.hpp>

namespace TradingEngine::Indicators {
class RSI : public IIndicator {
public:
  explicit RSI(std::size_t);
  virtual ~RSI() override = default;
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
  boost::circular_buffer<double> buffer_;
  const InputRequirements requirements_{{InputData::Close}};
  IndicatorId id_{};
  const double threshold_{50};
};

} // namespace TradingEngine::Indicators