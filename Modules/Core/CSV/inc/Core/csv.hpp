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

class CSVLineParser {
public:
  CSVRow operator()(std::string_view line, std::string_view delimiter) const;
};

class CSVReader {
public:
  class CSVReaderIterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = CSVRow;
    using difference_type = std::ptrdiff_t;
    using pointer = const CSVRow *;
    using reference = const CSVRow &;

    CSVReaderIterator() = default;
    explicit CSVReaderIterator(CSVReader &reader) : reader{&reader}, current{reader.get_next()} {};

    reference operator*() const noexcept { return *current; };
    pointer operator->() const noexcept { return &*current; };

    CSVReaderIterator &operator++() {
      if (!reader) {
        throw std::logic_error("Incrementing end iterator");
      }
      current = reader->get_next();
      return *this;
    };
    CSVReaderIterator operator++(int) {
      auto temp{*this};
      current = reader->get_next();
      return temp;
    };

    bool operator==(const CSVReaderIterator &other) const noexcept {
      return (reader == other.reader && current.has_value() == other.current.has_value());
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

public:
  using iterator = CSVReaderIterator;
  using sentinel = std::default_sentinel_t;

  explicit CSVReader(std::filesystem::path path, bool has_header = true,
                     std::string delimiter = ",");
  ~CSVReader() = default;
  CSVReader(CSVReader &&other) = default;
  CSVReader &operator=(CSVReader &&other) = default;
  bool operator!() const noexcept { return !csv_file; }
  iterator begin() {
    reset();
    return CSVReaderIterator(*this);
  };
  sentinel end() noexcept { return std::default_sentinel; }

  CSVReader(const CSVReader &other) = delete;
  CSVReader &operator=(const CSVReader &other) = delete;

  std::optional<CSVRow> get_next();

  void set_delimiter(std::string delimiter);
  void set_path(std::filesystem::path path, bool has_header);

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
  CSVLineParser line_parser;
  std::filesystem::path path;
  std::ifstream csv_file;
  std::string delimiter;
  CSVRow header;
};

inline bool operator==(std::default_sentinel_t s, const CSVReader::CSVReaderIterator &it) noexcept {
  return it == s;
}
inline bool operator!=(std::default_sentinel_t s, const CSVReader::CSVReaderIterator &it) noexcept {
  return it != s;
}

} // namespace TradingEngine::Core
