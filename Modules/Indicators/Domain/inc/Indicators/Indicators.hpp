#pragma once
#include <Core/Core.hpp>
#include <MarketData/candle.hpp>
#include <optional>
#include <vector>

namespace TradingEngine::Indicators {

struct IndicatorOutput {
  Core::Timestamp timestamp;
  double signal1;
  double signal2;
};

enum class InputData {
  Open,
  High,
  Low,
  Close,
  Volume,
};

enum class SignalType {
  ZeroCross,
  TwoLineCross,
  PriceCross,
};

using InputRequirements = std::vector<InputData>;

class IIndicator {
public:
  virtual ~IIndicator() = default;
  virtual std::optional<IndicatorOutput> update(const MarketData::Candle &) = 0;
  [[nodiscard]] virtual const InputRequirements &requirements() const = 0;
  [[nodiscard]] virtual SignalType signal_type() const noexcept = 0;
};

} // namespace TradingEngine::Indicators