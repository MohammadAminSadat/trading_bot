**Requirements Specification**

Project: NNFX Trading Engine

Version: 1.0

Status: Draft

---

# 1. Introduction

## 1.1 Purpose

The purpose of this document is to define the functional and non-functional requirements for the NNFX Trading Engine. These requirements describe the expected behavior of the system independently of its implementation details.

The system will initially support historical data collection, backtesting, and parameter optimization for the No Nonsense Forex (NNFX) methodology using 4-hour candles. The architecture shall remain extensible to support additional strategies, brokers, and markets.

---

# 2. Scope

The first production version of the system shall support:

- Downloading historical market data.
- Local storage of historical data.
- Technical indicator calculation.
- NNFX strategy evaluation.
- Historical back-testing.
- Performance analysis.
- Strategy optimization.
- Future integration with MetaTrader 5.

---

# 3. Functional Requirements

## FR-1 Historical Data

The system shall:

- Download historical OHLCV data from Yahoo Finance.
- Support configurable symbols.
- Support configurable date ranges.
- Validate downloaded data.
- Store downloaded data locally.

---

## FR-2 Market Data Repository

The system shall:

- Load historical data from local storage.
- Save new market data.
- Prevent duplicate candles.
- Retrieve data efficiently by symbol and date.

---

## FR-3 Technical Indicators

The system shall support:

- ATR
- EMA
- ADX
- RSI
- CCI
- MACD

Indicators shall be configurable through external configuration files.

---

## FR-4 Trading Strategy

The system shall implement the complete NNFX methodology, including:

- Baseline indicator
- Confirmation indicators
- Volume indicator
- ATR-based stop-loss
- Entry conditions
- Exit conditions
- Position sizing
- Risk management

---

## FR-5 Back-testing

The back-tester shall:

- Simulate trades chronologically.
- Prevent look-ahead bias.
- Record every simulated trade.
- Produce deterministic results for identical inputs.

---

## FR-6 Performance Metrics

The system shall calculate:

- Net profit
- Win rate
- Profit factor
- Maximum drawdown
- Average trade
- Average win
- Average loss
- Risk-reward ratio
- Sharpe ratio (future)
- Sortino ratio (future)

---

## FR-7 Optimization

The optimizer shall:

- Support parameter optimization.
- Execute multiple simulations automatically.
- Export optimization results.
- Support future walk-forward analysis.

---

## FR-8 Configuration

The system shall:

- Read configuration from JSON.
- Validate configuration values.
- Provide sensible default values.
- Report configuration errors.

---
## FR-9 Logging

The system shall:

- Log all important events.
- Support multiple log levels.
- Save logs to files.
- Display logs on the console.

---

## FR-10 Testing

The project shall include automated unit tests for all core components.

---

# 4. Non-Functional Requirements

## NFR-1 Performance

- Download historical data efficiently.
- Back-testing should support millions of candles.
- Indicator calculations should avoid unnecessary memory allocations.

---

## NFR-2 Reliability

The application shall:

- Handle invalid data gracefully.
- Recover from network failures.
- Detect corrupted data.
- Never silently ignore errors.

---

## NFR-3 Maintainability

The codebase shall:

- Follow modern C++20 practices.
- Follow the project's coding guidelines.
- Be modular.
- Be documented.
- Be testable.

---

## NFR-4 Portability

The project shall compile on:

- Linux (primary)
- Windows
- Raspberry Pi OS

---

## NFR-5 Scalability

The architecture shall support:

- Additional brokers.
- Additional trading strategies.
- Additional indicators.
- Additional financial instruments.
- Multiple simultaneous symbols.

---

## NFR-6 Security

The application shall:

- Never expose API credentials.
- Validate external input.
- Store configuration securely.

---

# 5. Assumptions

The following assumptions are made:
- Historical data is available from Yahoo Finance.
- Internet access is available when downloading data.
- Market data is assumed to be accurate.
- Back-testing uses completed candles only.

---

# 6. Constraints

- Programming language: C++20
- Build system: CMake
- Historical data provider: Yahoo Finance
- Local database: SQLite3
- Initial timeframe: 4-hour candles
- Initial strategy: No Nonsense Forex

---

# 7. Future Requirements

The architecture should support future additions including:

- MetaTrader 5 execution
- Live trading
- Paper trading
- Web dashboard
- Telegram notifications
- Multi-threaded optimization
- Multiple simultaneous strategies
- Walk-forward analysis
- Monte Carlo analysis
- Portfolio management

---

# 8. Success Criteria

The first production version will be considered successful when it can:

- Download historical market data.
- Store and reload data correctly.
- Execute deterministic back-tests.
- Produce accurate performance statistics.
- Optimize strategy parameters.
- Generate reproducible results.
- Operate autonomously without manual intervention.