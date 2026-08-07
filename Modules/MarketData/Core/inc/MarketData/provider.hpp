#pragma once
#include <concepts>
#include <optional>

#include "MarketData/candle.hpp"

namespace TradingEngine::MarketData {

template <typename T>
concept HistoricalProvider = requires(T& provider) {
  { provider.get_next() } -> std::same_as<std::optional<Candle>>;
};

} // namespace TradingEngine::MarketData
