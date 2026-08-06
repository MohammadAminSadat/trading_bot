#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace TradingEngine::Core {
using CSVRow = std::vector<std::string>;
class CSVParser {
public:
  explicit CSVParser(std::filesystem::path &&path, bool has_header = true,
                     std::string delimiter = ",");
  ~CSVParser() = default;
  CSVParser(CSVParser &&other) = default;
  CSVParser &operator=(CSVParser &&other) = default;

  CSVParser(const CSVParser &other) = delete;
  CSVParser &operator=(const CSVParser &other) = delete;

  std::optional<CSVRow> get_next();
  void open();
  bool is_open() { return csv_file.is_open(); };

  void set_delimiter(std::string delimiter);
  void set_path(std::filesystem::path &&path, bool has_header);

  void reset() { csv_file.seekg(0); };
  bool get_header_processed() const { return is_header_processed; }
  const std::string &get_delimiter() const { return delimiter; }
  const CSVRow &get_header() const;
  const CSVRow &get_header();

private:
  void parse_header();
  CSVRow parse_line(const std::string &line) noexcept;
  bool is_header_processed{false};
  bool has_header{false};
  std::filesystem::path path;
  std::ifstream csv_file;
  std::string delimiter;
  CSVRow header;
};

} // namespace TradingEngine::Core
