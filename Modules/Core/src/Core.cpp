#include "Core/Core.hpp"

namespace TradingEngine::Core {
TimeStamp make_timestamp(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t min,
                         int32_t sec, int32_t millisec) {
  std::chrono::year_month_day ymd{std::chrono::year{year},
                                  std::chrono::month{static_cast<unsigned>(month)},
                                  std::chrono::day{static_cast<unsigned>(day)}};

  std::chrono::sys_days datePoint = std::chrono::sys_days{ymd};

  auto timeOfDay = std::chrono::hours{hour} + std::chrono::minutes{min} +
                   std::chrono::seconds{sec} + std::chrono::milliseconds{millisec};

  return datePoint + timeOfDay;
}
} // namespace TradingEngine::Core