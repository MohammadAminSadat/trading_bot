#pragma once
#include <MarketData/candle.hpp>
#include <MarketData/provider.hpp>
#include <vector>

namespace TradingEngine::MarketData {

template <HistoricalRange Provider>
CandleSeries load_series(Provider &provider, TimeFrame time_frame = TimeFrame::Unknown) {
  std::vector<Candle> candles;

  for (const auto &candle : provider) {
    candles.push_back(candle);
  }

  return CandleSeries{std::move(candles), time_frame};
}

} // namespace TradingEngine::MarketData