#include "Core/csv.hpp"
namespace TradingEngine::Core {

CSVParser::CSVParser(std::ifstream &&file, bool has_header,
                     std::string delimiter)
    : csv_file{std::move(file)}, has_header{has_header},
      delimiter{std::move(delimiter)} {
  if (!csv_file.is_open()) {
    throw std::runtime_error("The file needs to be opend\n");
  }
}

const CSVRow &CSVParser::parse_header() {
  if (!has_header) {
    throw std::logic_error("CSV has no header");
  }

  if (is_header_processed) {
    return header;
  }

  std::string line;
  if (!std::getline(csv_file, line)) {
    return {};
  }
  is_header_processed = true;

  header = parse_line(line);
  return header;
}

std::optional<CSVRow> CSVParser::get_next() {
  if (has_header && !is_header_processed) {
    return parse_header();
  }
  std::string line;
  if (!std::getline(csv_file, line)) {
    return std::nullopt;
  }
  return parse_line(line);
}

std::vector<CSVRow> CSVParser::parse_csv() {
  reset();
  std::vector<CSVRow> data;
  std::string line;
  if (has_header && !is_header_processed) {
    parse_header();
  }

  while (std::getline(csv_file, line)) {
    data.emplace_back(parse_line(line));
  }

  if (data.empty()) {
    return std::vector<CSVRow>{};
  }
  return data;
}

void CSVParser::set_delimiter(std::string delimiter) {
  this->delimiter = std::move(delimiter);
}

void CSVParser::set_file(std::ifstream &&file, bool has_header_) {
  if (!csv_file.is_open()) {
    throw std::runtime_error("The file needs to be opend");
  }

  csv_file = std::move(file);
  has_header = has_header_;
  is_header_processed = false;

  header.clear();
}

CSVRow CSVParser::parse_line(const std::string &line) noexcept {
  CSVRow fields;
  size_t position{0};

  while (position <= line.length()) {
    size_t next = line.find(delimiter, position);
    if (next == std::string::npos) {
      fields.emplace_back(line.substr(position));
      break;
    }

    fields.emplace_back(line.substr(position, next - position));
    position = next + delimiter.length();
  }

  return fields;
}
const CSVRow &CSVParser::get_header() const { return header; }

} // namespace TradingEngine::Core
