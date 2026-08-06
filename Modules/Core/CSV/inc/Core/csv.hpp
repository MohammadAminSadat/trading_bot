#pragma once

#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace TradingEngine::Core {
using CSVRow = std::vector<std::string>;
class CSVParser {
public:
  explicit CSVParser(std::ifstream &&file, bool has_header = true,
                     std::string delimiter = ",");
  ~CSVParser() = default;
  CSVParser(CSVParser &&other) = default;
  CSVParser &operator=(CSVParser &&other) = default;

  CSVParser(const CSVParser &other) = delete;
  CSVParser &operator=(const CSVParser &other) = delete;

  const CSVRow &parse_header();
  std::optional<CSVRow> get_next();
  std::vector<CSVRow> parse_csv();

  void set_delimiter(std::string delimiter);
  void set_file(std::ifstream &&file, bool has_header);

  void reset() { csv_file.seekg(0); };
  bool get_header_processed() const { return is_header_processed; }
  const std::string &get_delimiter() const { return delimiter; }
  const CSVRow &get_header() const;

private:
  CSVRow parse_line(const std::string &line) noexcept;
  bool is_header_processed{false};
  bool has_header{false};
  std::ifstream csv_file;
  std::string delimiter;
  CSVRow header;
};

} // namespace TradingEngine::Core
