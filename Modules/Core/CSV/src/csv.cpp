#include "Core/csv.hpp"

#include <fstream>
#include <sstream>

namespace TradingEngine::Core {

CSV::CSV(std::filesystem::path path, std::string delimiter, bool has_header)
    : has_header{has_header}, delimiter{std::move(delimiter)} {
  if (!std::filesystem::exists(path)) {
    std::stringstream error;
    error << "file: " << path << " does not exist";
    throw std::runtime_error(error.str());
  }
  is_header_processed = !has_header;
  file.open(path);
  if (!file.is_open()) {
    std::stringstream error;
    error << "file: " << path << " could not be opened";
    throw std::runtime_error(error.str());
  }
  this->path = std::move(path);
}

CSV::CSV(CSV &&other) noexcept
    : has_header{other.has_header},
      is_header_processed{other.is_header_processed},
      path{std::move(other.path)},
      delimiter{std::move(other.delimiter)},
      file{std::move(other.file)} {
  other.has_header = false;
  other.is_header_processed = false;
}

CSV &CSV::operator=(CSV &&other) noexcept {
  if (this != &other) {
    has_header = other.has_header;
    is_header_processed = other.is_header_processed;
    path = std::move(other.path);
    delimiter = std::move(other.delimiter);
    file = std::move(other.file);
    other.has_header = false;
    other.is_header_processed = false;
  }
  return *this;
}

std::vector<std::string> CSV::read_header() {
  if (!has_header) {
    return {};
  }
  if (is_header_processed) {
    return {};
  }
  std::string line;
  if (!std::getline(file, line)) {
    return {};
  }
  is_header_processed = true;
  return parse_line(line);
}

std::vector<std::string> CSV::read_next() {
  std::string line;
  if (!std::getline(file, line)) {
    return {};
  }
  return parse_line(line);
}

std::vector<std::string> CSV::parse_line(const std::string &line) {
  std::vector<std::string> fields;
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
