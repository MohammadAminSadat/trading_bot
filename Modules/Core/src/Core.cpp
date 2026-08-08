#include "Core/Core.hpp"

namespace TradingEngine::Core {
Timestamp make_timestamp(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t min,
                         int32_t sec, int32_t millisec) {
  std::chrono::year_month_day ymd{std::chrono::year{year},
                                  std::chrono::month{static_cast<unsigned>(month)},
                                  std::chrono::day{static_cast<unsigned>(day)}};
  if (!ymd.ok() || hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59 ||
      millisec < 0 || millisec > 999) {
    throw std::invalid_argument("Invalid date/time components");
  }

  std::chrono::sys_days datePoint = std::chrono::sys_days{ymd};

  auto timeOfDay = std::chrono::hours{hour} + std::chrono::minutes{min} +
                   std::chrono::seconds{sec} + std::chrono::milliseconds{millisec};

  return datePoint + timeOfDay;
}

int32_t parse_int(std::string_view sv) {
  int32_t value{};
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

  if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
    throw std::invalid_argument("Invalid integer.");
  }

  return value;
};

double parse_double(std::string_view sv) {
  double value{};
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

  if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
    throw std::invalid_argument("Invalid double.");
  }

  return value;
}

} // namespace TradingEngine::Core