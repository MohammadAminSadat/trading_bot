#pragma once
#include <chrono>
#include <cstdint>
#include <string_view>
namespace TradingEngine::Core {
using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

Timestamp make_timestamp(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t min,
                         int32_t sec = 0, int32_t millisec = 0);

int32_t parse_int(std::string_view);
double parse_double(std::string_view);

} // namespace TradingEngine::Core
