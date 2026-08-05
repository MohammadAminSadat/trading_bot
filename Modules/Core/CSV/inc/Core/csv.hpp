#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace TradingEngine::Core {
using CSVRow = std::vector<std::string>;
class CSV {
public:
  explicit CSV(std::filesystem::path path, std::string delimiter = ",", bool has_header = false);
  virtual ~CSV();
  CSV(CSV &&other) noexcept;
  CSV &operator=(CSV &&other) noexcept;

  CSV(const CSV &other) = delete;
  CSV &operator=(const CSV &other) = delete;

  CSVRow parse_header();
  CSVRow read_next();
  std::vector<CSVRow> parse_csv();

  void set_path(std::filesystem::path &path, bool has_header = false);
  void set_delimiter(std::string &delimiter);

private:
  CSVRow parse_line(const std::string &line);

  bool has_header{false};
  bool is_header_processed{false};
  bool is_cached{false};
  std::vector<CSVRow> cache;
  std::filesystem::path path;
  std::string delimiter;
  std::ifstream csv_file;
  CSVRow header;
};

} // namespace TradingEngine::Core
