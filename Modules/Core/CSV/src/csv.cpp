#include "Core/csv.hpp"

#include <fstream>
#include <sstream>

namespace TradingEngine::Core {

CSV::CSV(std::filesystem::path path, std::string delimiter, bool has_header)
    : has_header{has_header}, delimiter{std::move(delimiter)} {
  if (!std::filesystem::exists(path)) {
    std::stringstream error;
    error << "csv_file: " << path << " does not exist";
    throw std::runtime_error(error.str());
  }
  is_header_processed = !has_header;
  csv_file.open(path);
  if (!csv_file.is_open()) {
    std::stringstream error;
    error << "file: " << path << " could not be opened";
    throw std::runtime_error(error.str());
  }
  this->path = std::move(path);
}

CSV::~CSV() {
  if (csv_file.is_open()) {
    csv_file.close();
  }
}
CSV::CSV(CSV &&other) noexcept
    : has_header{other.has_header},
      is_header_processed{other.is_header_processed},
      path{std::move(other.path)},
      delimiter{std::move(other.delimiter)},
      csv_file{std::move(other.csv_file)} {
  other.has_header = false;
  other.is_header_processed = false;
}

CSV &CSV::operator=(CSV &&other) noexcept {
  if (this != &other) {
    has_header = other.has_header;
    is_header_processed = other.is_header_processed;
    path = std::move(other.path);
    delimiter = std::move(other.delimiter);
    csv_file = std::move(other.csv_file);
    other.has_header = false;
    other.is_header_processed = false;
  }
  return *this;
}

CSVRow CSV::parse_header() {
  if (!has_header) {
    return {};
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

CSVRow CSV::read_next() {
  std::string line;
  if (!std::getline(csv_file, line)) {
    return {};
  }
  return parse_line(line);
}

std::vector<CSVRow> CSV::parse_csv() {
  std::vector<CSVRow> data;
  if (!is_header_processed) {
    parse_header();
  }
  if (is_cached) {
    return cache;
  }
  std::string line;
  while (std::getline(csv_file, line)) {
    CSVRow row{parse_line(line)};
    data.emplace_back(row);
  }
  is_cached = true;
  cache = data;
  return data;
}

void CSV::set_path(std::filesystem::path &path_, bool has_header_) {
  if (csv_file.is_open()) {
    csv_file.close();
  }
  if (path_ == path) {
    return;
  }
  csv_file.open(path);
  is_cached = false;
  cache.empty();
  is_header_processed = false;
  header.empty();
  has_header = has_header_;
}

void CSV::set_delimiter(std::string &delimiter_) {
  delimiter = delimiter_;
}

CSVRow CSV::parse_line(const std::string &line) {
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

} // namespace TradingEngine::Core
