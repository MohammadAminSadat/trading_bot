#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace TradingEngine::Core {
using CSVRow = std::vector<std::string>;
class CSVParser {
public:
  explicit CSVParser(std::filesystem::path path, std::string delimiter = ",",
                     bool has_header = false);
  virtual ~CSVParser();
  CSVParser(CSVParser &&other) noexcept;
  CSVParser &operator=(CSVParser &&other) noexcept;

  CSVParser(const CSVParser &other) = delete;
  CSVParser &operator=(const CSVParser &other) = delete;

  CSVRow parse_header();
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
