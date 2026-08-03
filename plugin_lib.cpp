// ============================================================
// plugin_lib.cpp  —  Shared library loaded at runtime via Boost.DLL
// Build: shared library (libplugin.so / plugin.dll)
// ============================================================

#include <string>
#include <vector>
#include <algorithm>
#include <numeric>

extern "C" {

__attribute__((visibility("default")))
const char* plugin_name()
{
    return "MovingAverage";
}

__attribute__((visibility("default")))
double compute_sma(const double* prices, int count)
{
    if (count <= 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < count; ++i)
        sum += prices[i];
    return sum / count;
}

__attribute__((visibility("default")))
double compute_max_drawdown(const double* prices, int count)
{
    if (count <= 1) return 0.0;
    double peak = prices[0];
    double max_dd = 0.0;
    for (int i = 1; i < count; ++i)
    {
        if (prices[i] > peak)
            peak = prices[i];
        double dd = (peak - prices[i]) / peak;
        if (dd > max_dd) max_dd = dd;
    }
    return max_dd;
}

} // extern "C"
