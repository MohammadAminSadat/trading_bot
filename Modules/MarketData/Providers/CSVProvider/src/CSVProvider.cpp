#include "MarketData/CSVProvider.hpp"

namespace TradingEngine::MarketData {
Core::Timestamp default_timestamp_parser(std::string_view date) {
  // data formate 2010-07-20 20:00
  if (date.size() != 16 || date[4] != '-' || date[7] != '-' || date[10] != ' ' || date[13] != ':') {
    throw std::runtime_error("Expected timestamp format: YYYY-MM-DD HH:MM");
  }

  int32_t year{Core::parse_int(date.substr(0, 4))};
  int32_t month{Core::parse_int(date.substr(5, 2))};
  int32_t day{Core::parse_int(date.substr(8, 2))};
  int32_t hour{Core::parse_int(date.substr(11, 2))};
  int32_t minute{Core::parse_int(date.substr(14, 2))};
  return Core::make_timestamp(year, month, day, hour, minute);
}

CSVProvider::CSVProvider(CSVImporterConfiguration configs)
    : configs{std::move(configs)},
      reader(this->configs.path, this->configs.has_header, this->configs.delimiter) {
  auto data{reader.get_next()};
  if (!data) {
    throw std::runtime_error("CSV file contains no data");
  }

  if (!configs.column_mapping.is_in_bounds(data->size())) {
    throw std::runtime_error("Column mapping is out of bounds");
  }
  reader.reset();
}

std::optional<Candle> CSVProvider::get_next() {
  std::optional<Core::CSVRow> row{reader.get_next()};
  if (!row.has_value()) {
    return std::nullopt;
  }
  const Core::CSVRow &data{row.value()};

  Core::Timestamp timestamp{configs.timestamp_parser(data.at(configs.column_mapping.timestamp))};
  Price open{Core::parse_double(data.at(configs.column_mapping.open))};
  Price high{Core::parse_double(data.at(configs.column_mapping.high))};
  Price low{Core::parse_double(data.at(configs.column_mapping.low))};
  Price close{Core::parse_double(data.at(configs.column_mapping.close))};
  std::optional<int64_t> volume{std::nullopt};
  if (configs.column_mapping.volume.has_value()) {
    volume = Core::parse_int(data.at(configs.column_mapping.volume.value()));
  }
  Candle candle{open, high, low, close, timestamp, volume};
  if (!candle.validate()) {
    throw std::runtime_error("Candle is invalid");
  }

  return candle;
}

} // namespace TradingEngine::MarketData