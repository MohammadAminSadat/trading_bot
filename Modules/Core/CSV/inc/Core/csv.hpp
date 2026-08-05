#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace TradingEngine::Core {

class CSV {
public:
  explicit CSV(std::filesystem::path path, std::string delimiter = ",", bool has_header = false);
  CSV(CSV &&other) noexcept;
  CSV &operator=(CSV &&other) noexcept;

  CSV(const CSV &other) = delete;
  CSV &operator=(const CSV &other) = delete;

  std::vector<std::string> read_header();
  std::vector<std::string> read_next();

private:
  std::vector<std::string> parse_line(const std::string &line);

  bool has_header{false};
  bool is_header_processed{false};
  std::filesystem::path path;
  std::string delimiter;
  std::ifstream file;
};

} // namespace TradingEngine::Core
