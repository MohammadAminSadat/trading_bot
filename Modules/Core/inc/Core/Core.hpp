#pragma once
#include <chrono>
#include <cstdint>

namespace TradingEngine::Core {
using TimeStamp = std::chrono::sys_time<std::chrono::milliseconds>;

TimeStamp make_timestamp(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t min,
                         int32_t sec = 0, int32_t millisec = 0);
} // namespace TradingEngine::Core
