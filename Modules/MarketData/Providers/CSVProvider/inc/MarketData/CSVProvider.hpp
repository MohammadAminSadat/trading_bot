#pragma once
#include <Core/Core.hpp>
#include <Core/csv.hpp>
#include <MarketData/candle.hpp>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

namespace TradingEngine::MarketData {

using TimestampParser = std::function<Core::Timestamp(std::string_view)>;
inline Core::Timestamp default_timestamp_parser(std::string_view);
struct ColumnMapping {
  size_t timestamp;
  size_t open;
  size_t high;
  size_t low;
  size_t close;
  std::optional<size_t> volume;

  bool is_in_bounds(size_t row_size) const noexcept {
    return timestamp < row_size && open < row_size && high < row_size && low < row_size &&
           close < row_size && (!volume.has_value() || volume.value() < row_size);
  }
};

struct CSVImporterConfiguration {
  std::filesystem::path path;
  TimestampParser timestamp_parser{default_timestamp_parser};

  ColumnMapping column_mapping{.timestamp{0}, .open{1}, .high{2}, .low{3}, .close{4}, .volume{5}};

  std::string delimiter{","};
  bool has_header{true};
};

class CSVProvider {
public:
  class Iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Candle;
    using difference_type = std::ptrdiff_t;
    using pointer = const Candle *;
    using reference = const Candle &;

    Iterator() = default;
    explicit Iterator(CSVProvider &provider) : provider{&provider}, current{provider.get_next()} {};

    reference operator*() const noexcept { return *current; };
    pointer operator->() const noexcept { return &*current; };

    Iterator &operator++() {
      if (!provider) {
        throw std::logic_error("Incrementing end iterator");
      }
      current = provider->get_next();
      return *this;
    };
    Iterator operator++(int) {
      if (!provider) {
        throw std::logic_error("Incrementing end iterator");
      }
      auto temp{*this};
      current = provider->get_next();
      return temp;
    };

    bool operator==(std::default_sentinel_t) const noexcept {
      return (provider == nullptr || !current.has_value());
    }
    bool operator!=(std::default_sentinel_t s) const noexcept { return !(*this == s); };

  private:
    std::optional<Candle> current;
    CSVProvider *provider{nullptr};
  };

  using iterator = Iterator;
  using sentinel = std::default_sentinel_t;

  CSVProvider(CSVImporterConfiguration configs);
  ~CSVProvider() = default;
  std::optional<Candle> get_next();
  void reset() { reader.reset(); };
  iterator begin() {
    reset();
    return Iterator(*this);
  };
  sentinel end() noexcept { return std::default_sentinel; }

private:
  CSVImporterConfiguration configs;
  Core::CSVReader reader;
};

inline bool operator==(std::default_sentinel_t s, const CSVProvider::Iterator &it) noexcept {
  return it == s;
}
inline bool operator!=(std::default_sentinel_t s, const CSVProvider::Iterator &it) noexcept {
  return it != s;
}

} // namespace TradingEngine::MarketData
