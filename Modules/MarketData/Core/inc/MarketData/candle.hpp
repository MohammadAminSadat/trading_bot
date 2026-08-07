#pragma once

#include <Core/Core.hpp>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

namespace TradingEngine::MarketData {

using Price = double;

enum class TimeFrame {
  Unknown,
  M10,
  M30,
  H1,
  H2,
  H4,
  H12,
  D1,
  W1,
};

struct Candle {
  Price open;
  Price high;
  Price low;
  Price close;
  std::optional<std::int64_t> volume;
  Core::Timestamp timestamp;

  Candle(Price open, Price high, Price low, Price close, Core::Timestamp ts,
         std::optional<std::int64_t> vol = std::nullopt)
      : open(open), high(high), low(low), close(close), timestamp(ts), volume(vol) {}
  [[nodiscard]]
  bool validate() const noexcept {
    return std::isfinite(open) && std::isfinite(high) && std::isfinite(low) &&
           std::isfinite(close) && high >= open && high >= close && high >= low && low <= open &&
           low <= close && (!volume || *volume >= 0);
  }
  bool is_bullish() const noexcept { return (close >= open); }
  bool is_bearish() const noexcept { return close < open; }
  Price typical_price() const noexcept { return ((low + high + close) / 3); }
  Price median_price() const noexcept { return (high + low) / 2; }
  Price weighted_close_price() const noexcept { return ((low + high + 2 * close) / 4); }
};

class CandleSeries {
public:
  CandleSeries(std::vector<Candle> candles, TimeFrame time_frame)
      : candles{std::move(candles)}, time_frame{time_frame} {}
  explicit CandleSeries(TimeFrame time_frame) : time_frame{time_frame} {};
  std::size_t size() const noexcept { return candles.size(); }
  auto begin() noexcept { return candles.begin(); }
  auto end() noexcept { return candles.end(); }
  auto begin() const noexcept { return candles.begin(); }
  auto end() const noexcept { return candles.end(); }
  bool empty() const noexcept { return candles.empty(); }
  void reserve(std::size_t size) { candles.reserve(size); }
  void push_back(Candle candle) { candles.push_back(std::move(candle)); }
  void emplace_back(Price open, Price high, Price low, Price close, Core::Timestamp time_stamp,
                    std::optional<std::int64_t> volume = std::nullopt) {
    candles.emplace_back(open, high, low, close, time_stamp, volume);
  }
  Candle &operator[](size_t index) { return candles[index]; }
  const Candle &operator[](size_t index) const { return candles[index]; }
  const Candle &latest() const { return candles.back(); }
  [[nodiscard]]
  TimeFrame get_time_frame() const noexcept {
    return time_frame;
  }

private:
  std::vector<Candle> candles;
  TimeFrame time_frame;
};

} // namespace TradingEngine::MarketData