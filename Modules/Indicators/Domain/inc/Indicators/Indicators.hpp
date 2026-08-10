#pragma once
#include <Core/Core.hpp>
#include <MarketData/candle.hpp>
#include <cstdint>
#include <optional>
#include <vector>

namespace TradingEngine::Indicators {

using IndicatorId = uint64_t;

struct SignalOutput {
  double signal1;
  double signal2;
};

struct IndicatorResult {
  IndicatorId id;
  Core::Timestamp timestamp;
  SignalOutput output;
};

enum class InputData {
  Custom,
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
  ThresholdCross,
};

using InputRequirements = std::vector<InputData>;

class IIndicator {
public:
  virtual ~IIndicator() = default;
  virtual std::optional<IndicatorResult> update(const MarketData::Candle &) = 0;
  [[nodiscard]] virtual const InputRequirements &requirements() const = 0;
  [[nodiscard]] virtual SignalType signal_type() const noexcept = 0;
};

} // namespace TradingEngine::Indicators