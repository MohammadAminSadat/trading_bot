#include "MarketData/CSVImporter.hpp"

namespace TradingEngine::MarketData {

CSVImporter::CSVImporter(ImporterConfiguration&& configs) : configs{std::move(configs)} {
}

} // namespace TradingEngine::MarketData