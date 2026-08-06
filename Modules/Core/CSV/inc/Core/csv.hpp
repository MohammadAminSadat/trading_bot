#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace TradingEngine::Core {
using CSVRow = std::vector<std::string>;
class CSVReader;

class CSVReaderIterator {
public:
  using iterator_category = std::input_iterator_tag;
  using value_type = CSVRow;
  using difference_type = std::ptrdiff_t;
  using pointer = CSVRow *;
  using reference = CSVRow &;

  CSVReaderIterator() = default;
  explicit CSVReaderIterator(CSVReader &reader);

  CSVRow &operator*() noexcept { return *current; };
  CSVRow *operator->() noexcept { return &*current; };
  CSVRow const &operator*() const noexcept { return *current; };
  CSVRow const *operator->() const noexcept { return &*current; };

  CSVReaderIterator &operator++();
  CSVReaderIterator operator++(int);

  bool operator==(const CSVReaderIterator &other) const noexcept {
    return (!current.has_value() && !other.current.has_value());
  }
  bool operator!=(const CSVReaderIterator &other) const noexcept { return !(*this == other); }

  bool operator==(std::default_sentinel_t) const noexcept {
    return (reader == nullptr || !current.has_value());
  }
  bool operator!=(std::default_sentinel_t s) const noexcept { return !(*this == s); };

private:
  CSVReader *reader{nullptr};
  std::optional<CSVRow> current{std::nullopt};
};

inline bool operator==(std::default_sentinel_t s, const CSVReaderIterator &it) noexcept {
  return it == s;
}
inline bool operator!=(std::default_sentinel_t s, const CSVReaderIterator &it) noexcept {
  return it != s;
}

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
  bool operator!() const noexcept { return !csv_file; }
  auto begin() { return CSVReaderIterator(*this); };
  auto end() { return CSVReaderIterator{}; }

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

inline CSVReaderIterator ::CSVReaderIterator(CSVReader &reader) : reader{&reader} {
  current = reader.get_next();
}

inline CSVReaderIterator &CSVReaderIterator ::operator++() {
  current = reader->get_next();
  return *this;
}

inline CSVReaderIterator CSVReaderIterator ::operator++(int) {
  auto temp{*this};
  current = reader->get_next();
  return temp;
}

} // namespace TradingEngine::Core
