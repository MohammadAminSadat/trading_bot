#pragma once
#include <Indicators/EMA.hpp>
#include <Indicators/Indicators.hpp>

namespace TradingEngine::Indicators {
class MACD : public IIndicator {
  MACD(std::size_t, std::size_t, std::size_t);
  virtual ~MACD() override = default;
  virtual std::optional<IndicatorResult> update(const MarketData::Candle &) override;
  virtual std::optional<SignalOutput> update(double) noexcept;
  [[nodiscard]] virtual const InputRequirements &requirements() const override {
    return requirements_;
  };
  [[nodiscard]] virtual SignalType signal_type() const noexcept override {
    return SignalType::TwoLineCross;
  };
  IndicatorId indicator_id() { return id_; }

private:
  const std::size_t slow_period_;
  const std::size_t fast_period_;
  const std::size_t signal_period_;
  EMA slow_ema_;
  EMA fast_ema_;
  EMA signal_ema_;
  const InputRequirements requirements_{{InputData::Close}};
  IndicatorId id_{};
};

} // namespace TradingEngine::Indicators