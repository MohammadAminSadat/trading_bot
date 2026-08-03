#pragma once

#include <chrono>
#include <vector>
#include <cstdint>

namespace TradingEngine {
namespace MarketData {

using TimeStamp = std::chrono::sys_time<std::chrono::seconds>;
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
  std::int64_t volume;
  TimeFrame time_frame;
  TimeStamp time_stamp;

  Candle(Price open, Price high, Price low, Price close, std::uint64_t volume,
         TimeFrame time_frame, TimeStamp time_stamp)
      : open{open}, high{high}, low{low}, close{close}, volume{volume},
        time_frame{time_frame}, time_stamp{time_stamp} {};

  [[nodiscard]] bool validate() const noexcept {
    return (high >= open) && (high >= close) && (high >= low) &&
           (low <= close) && (low <= open) && (volume >= 0);
  }
  bool is_bullish() const noexcept { return (close >= open); }
  bool is_bearish() const noexcept { return close < open; }
  Price typical_price() const noexcept { return ((low + high + close) / 3); }
  Price median_price() const noexcept { return (high + low) / 2; }
  Price weighted_close_price() const noexcept {
    return ((low + high + 2 * close) / 4);
  }
};

class CandleSeries {
public:
  CandleSeries(std::vector<Candle> candles) : candles{std::move(candles)} {}
  size_t size() { return candles.size(); }
  auto begin() { return candles.begin(); }
  auto end() { return candles.end(); }
  auto begin() const { return candles.begin(); }
  auto end() const { return candles.end(); }
  bool empty() const noexcept { return candles.empty(); }
  void reserve(std::size_t size) { candles.reserve(size); }
  void push_back(Candle candle) { candles.push_back(std::move(candle)); }
  Candle &operator[](size_t index) { return candles[index]; }
  const Candle &operator[](size_t index) const { return candles[index]; }
  const Candle &latest() const { return candles.back(); }

private:
  std::vector<Candle> candles;
};

} // namespace MarketData
} // namespace TradingEngine