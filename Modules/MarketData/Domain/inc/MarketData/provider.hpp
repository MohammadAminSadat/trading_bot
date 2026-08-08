#pragma once
#include <concepts>
#include <optional>
#include <ranges>

#include "MarketData/candle.hpp"

namespace TradingEngine::MarketData {

template <typename T>
concept HistoricalProvider = requires(T& provider) {
  { provider.get_next() } -> std::same_as<std::optional<Candle>>;
};

template <typename T>
concept HistoricalRange =
    std::ranges::input_range<T> && std::same_as<std::ranges::range_value_t<T>, Candle>;

} // namespace TradingEngine::MarketData
