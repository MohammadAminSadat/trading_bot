#include "Core/csv.hpp"

TradingEngine::Core::CSVRow TradingEngine::Core::CSVLineParser::operator()(
    std::string_view line, std::string_view delimiter) const {
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

namespace TradingEngine::Core {

CSVReader::CSVReader(std::filesystem::path path_, bool has_header, std::string delimiter)
    : path{path_}, has_header{has_header}, delimiter{std::move(delimiter)} {
  if (!std::filesystem::exists(path)) {
    std::stringstream error;
    error << "File: " << path.string() << " does not exist";
    throw std::runtime_error(error.str());
  }
}

void CSVReader::parse_header() {
  if (!is_open()) {
    open();
  }
  std::string line;
  if (!std::getline(csv_file, line)) {
    throw std::runtime_error("Reading header error!");
  }
  is_header_processed = true;

  header = parse_line(line);
}

std::optional<CSVRow> CSVReader::get_next() {
  if (!is_open()) {
    open();
  }
  if (has_header && !is_header_processed) {
    parse_header();
  }
  std::string line;
  if (!std::getline(csv_file, line)) {
    return std::nullopt;
  }
  return parse_line(line);
}

void CSVReader::open() {
  if (!is_open()) {
    csv_file.open(path);
  }
  if (!csv_file) {
    std::stringstream error;
    error << "File: " << path.string() << " does not exist";
    throw std::runtime_error(error.str());
  }
}
void CSVReader::set_delimiter(std::string delimiter) {
  this->delimiter = std::move(delimiter);
}

void CSVReader::set_path(std::filesystem::path path_, bool has_header_) {
  if (!std::filesystem::exists(path_)) {
    std::stringstream error;
    error << "File: " << path_.string() << " does not exist";
    throw std::runtime_error(error.str());
  }
  if (is_open()) {
    csv_file.close();
  }
  path = path_;
  has_header = has_header_;
  is_header_processed = false;
  header.clear();
  csv_file.clear();
}

void CSVReader::reset() {
  if (!csv_file) {
    return;
  }
  csv_file.clear();
  csv_file.seekg(0);
  is_header_processed = false;
  header.clear();
}

CSVRow CSVReader::parse_line(std::string &line) noexcept {
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
  return line_parser(line, delimiter);
}

const CSVRow &CSVReader::get_header() {
  if (!has_header) {
    throw std::logic_error("File Does not have header");
  }
  if (!is_header_processed) {
    parse_header();
  }
  return header;
}
const CSVRow &CSVReader::get_header() const {
  if (!has_header) {
    throw std::logic_error("File Does not have header");
  }
  return header;
}

} // namespace TradingEngine::Core
