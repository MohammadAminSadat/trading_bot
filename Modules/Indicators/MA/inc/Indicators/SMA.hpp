#pragma once
#include <Indicators/Indicators.hpp>
#include <boost/circular_buffer.hpp>

namespace TradingEngine::Indicators {

class SMA : public IIndicator {
public:
  SMA(std::size_t);
  virtual ~SMA() override = default;
  virtual std::optional<IndicatorResult> update(const MarketData::Candle &) override;
  virtual std::optional<SignalOutput> update(double) noexcept;
  [[nodiscard]] virtual const InputRequirements &requirements() const override {
    return requirements_;
  };
  [[nodiscard]] virtual SignalType signal_type() const noexcept override {
    return SignalType::PriceCross;
  };
  IndicatorId indicator_id() { return id; }

private:
  const std::size_t period{14};
  double sum_{};
  const InputRequirements requirements_{{InputData::Close}};
  boost::circular_buffer<double> cache;
  IndicatorId id{};
};
} // namespace TradingEngine::Indicators