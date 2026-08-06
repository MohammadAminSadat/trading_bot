#pragma once
#include <Core/Core.hpp>
#include <Core/csv.hpp>
#include <MarketData/candle.hpp>
#include <cstdint>
#include <functional>
#include <regex>
#include <sstream>
#include <string>

namespace TradingEngine::MarketData {

using TimeStepStrategy = std::function<Core::TimeStamp(const std::string &)>;

Core::TimeStamp default_strategy(const std::string &date) {
  const std::regex date_pattern{R"((\d{4})-(\d{2})-(\d{2}) (\d{2}):(\d{2}))"};
  std::smatch matches;
  if (!std::regex_search(date, matches, date_pattern)) {
    std::stringstream error;
    error << "Error in " << date << " format";
    throw std::runtime_error(error.str());
  }
  int32_t year{static_cast<int32_t>(std::stoi(matches[1]))};
  int32_t month{static_cast<int32_t>(std::stoi(matches[2]))};
  int32_t day{static_cast<int32_t>(std::stoi(matches[3]))};
  int32_t hour{static_cast<int32_t>(std::stoi(matches[4]))};
  int32_t minuet{static_cast<int32_t>(std::stoi(matches[5]))};

  return Core::make_timestamp(year, month, day, hour, minuet);
}

class ImporterConfiguration {
public:
  void set_time_stamp_strategy(TimeStepStrategy strategy = default_strategy) noexcept {
    time_stamp_strategy = strategy;
  };
  TimeStepStrategy time_stamp_strategy;

private:
};

class CSVImporter {
public:
  CSVImporter(ImporterConfiguration &&configs);
  ~CSVImporter() = default;

private:
  ImporterConfiguration configs;
};

} // namespace TradingEngine::MarketData