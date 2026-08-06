#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace TradingEngine::Core {
using CSVRow = std::vector<std::string>;

class CSVLineParser {
public:
  CSVRow operator()(std::string_view line, std::string_view delimiter) const;
};

class CSVReader {
public:
  explicit CSVReader(std::filesystem::path path, bool has_header = true,
                     std::string delimiter = ",");
  ~CSVReader() = default;
  CSVReader(CSVReader &&other) = default;
  CSVReader &operator=(CSVReader &&other) = default;

  CSVReader(const CSVReader &other) = delete;
  CSVReader &operator=(const CSVReader &other) = delete;

  std::optional<CSVRow> get_next();

  void set_delimiter(std::string delimiter);
  void set_path(std::filesystem::path &&path, bool has_header);

  void reset();
  bool get_header_processed() const { return is_header_processed; }
  const std::string &get_delimiter() const { return delimiter; }
  const CSVRow &get_header() const;
  const CSVRow &get_header();

private:
  void open();
  bool is_open() const noexcept { return csv_file.is_open(); };
  void parse_header();
  CSVRow parse_line(std::string &line) noexcept;
  bool is_header_processed{false};
  bool has_header{false};
  std::filesystem::path path;
  std::ifstream csv_file;
  std::string delimiter;
  CSVRow header;
};

} // namespace TradingEngine::Core
