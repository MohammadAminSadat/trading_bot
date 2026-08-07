#pragma once
#include <Core/Core.hpp>
#include <Core/csv.hpp>
#include <MarketData/candle.hpp>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>

namespace TradingEngine::MarketData {

using TimestampParser = std::function<Core::Timestamp(std::string_view)>;
Core::Timestamp default_timestamp_parser(std::string_view);
struct ColumnMapping {
  size_t timestamp;
  size_t open;
  size_t high;
  size_t low;
  size_t close;
  std::optional<size_t> volume;

  bool is_in_bounds(size_t row_size) const noexcept {
    return timestamp < row_size && open < row_size && high < row_size && low < row_size &&
           close < row_size && (!volume.has_value() || volume.value() < row_size);
  }
};

struct CSVImporterConfiguration {
  std::filesystem::path path;
  TimestampParser timestamp_parser{default_timestamp_parser};

  ColumnMapping column_mapping{.timestamp{0}, .open{1}, .high{2}, .low{3}, .close{4}, .volume{5}};

  std::string delimiter{","};
  bool has_header{true};
};

class CSVProvider {
public:
  CSVProvider(CSVImporterConfiguration configs);
  ~CSVProvider() = default;
  std::optional<Candle> get_next();

private:
  CSVImporterConfiguration configs;
  Core::CSVReader reader;
};

} // namespace TradingEngine::MarketData
