# NNFX Trading Engine

A **modular algorithmic trading platform** built in modern C++20 — designed as
both a reliable automated trading system and a deliberate exercise in
professional software engineering.

## Why this project exists

This project serves two goals:

1. **Build a trading engine** that implements the **No Nonsense Forex (NNFX)**
   methodology, capable of downloading historical data, calculating indicators,
   running deterministic back-tests, optimizing parameters, and eventually
   executing trades through MetaTrader 5 on a Raspberry Pi.

2. **Master modern C++** by applying clean architecture, dependency injection,
   test-driven development, and CI to a real, non-trivial application. Every
   phase teaches new engineering skills — not just adds features.

The project is written as if it were a **commercial-grade system**: modular,
tested, documented, and designed for long-term maintainability.

---

## Architecture

```text
                   ┌──────────────┐
                   │  Strategies  │
                   └──────┬───────┘
                          │
          ┌───────────────┼───────────────┐
          ▼               ▼               ▼
   ┌────────────┐  ┌────────────┐  ┌──────────────┐
   │ Indicators │  │ BackTesting│  │ Optimization │
   └─────┬──────┘  └─────┬──────┘  └──────┬───────┘
         │               │                │
         └───────────────┼────────────────┘
                         ▼
                 ┌──────────────┐
                 │  MarketData  │
                 └──────┬───────┘
                        │
         ┌──────────────┼──────────────┐
         ▼              ▼              ▼
   ┌──────────┐  ┌──────────┐  ┌────────────┐
   │  Storage │  │Providers │  │    Core     │
   │ (SQLite) │  │(Yahoo…)  │  │ (types/CSV) │
   └──────────┘  └──────────┘  └────────────┘
```

Modules are loosely coupled libraries exposed as CMake targets under the
`TradingEngine` namespace. The engine flows from raw market data through
indicators and strategies to back-testing and optimization.

---

## Project structure

```text
trading_bot/
├── App/
│   └── main.cpp              demo application
├── Modules/
│   ├── Core/
│   │   └── CSV/              CSV parser with C++20 input iterator
│   ├── MarketData/
│   │   └── Core/             Candle, CandleSeries, TimeFrame types
│   ├── Indicators/           indicator framework (planned)
│   ├── Strategies/           strategy definitions (planned)
│   ├── BackTesting/          deterministic back-tester (planned)
│   ├── Optimization/         parameter optimization (planned)
│   ├── Reporting/            performance metrics (planned)
│   └── Execution/            broker integration (planned)
├── docs/
│   ├── requirements.md       functional and non-functional requirements
│   ├── project_vision.md     design philosophy and long-term goals
│   ├── technology.md         technology stack decisions
│   └── future_extentions.md  roadmap beyond v1.0
├── cmake/                    CMake helper modules (clang-format)
├── database/                 historical CSV data (gitignored)
└── .github/workflows/ci.yml  CI pipeline
```

---

## What is built so far

| Module | Status |
|--------|--------|
| CSV parser (`Modules/Core/CSV`) | **Complete** — `CSVLineParser`, `CSVReader`, `CSVReaderIterator`, comprehensive tests, readme |
| Market data types (`MarketData/Core`) | **Complete** — `Candle`, `CandleSeries`, `TimeFrame`, unit tests |
| Build system | **Complete** — CMake, C++20, FetchContent (spdlog, Eigen, GoogleTest), Boost |
| CI pipeline | **Complete** — GitHub Actions, Debug/Release × gcc/clang, test + format check |
| Indicators | Planned |
| Back-testing engine | Planned |
| Strategy implementation | Planned |
| Optimization engine | Planned |
| MetaTrader 5 integration | Planned |

---

## Build and test

### Prerequisites

- **C++20 compiler** (GCC 14 or Clang 18)
- **CMake** ≥ 3.25
- **Boost** (headers + compiled libraries; the CI uses `libboost-all-dev`)

All other dependencies (GoogleTest, spdlog, Eigen) are fetched automatically via
CMake's `FetchContent`.

### Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

### Run the demo

```bash
./build/App/TradingEngine
```

### Format

```bash
cmake --build build --target format
```

---

## Technology stack

| Category | Choice |
|----------|--------|
| Language | C++20 |
| Build system | CMake |
| Compiler | GCC 14 / Clang 18 |
| Testing | GoogleTest + GoogleMock |
| Formatting | clang-format |
| Logging | spdlog |
| JSON configuration | nlohmann/json |
| Linear algebra | Eigen |
| Database | SQLite3 |
| Networking | Boost.Beast + Boost.Asio |
| CI | GitHub Actions |

---

## Design principles

- **Separation of concerns** — each module has a single responsibility
- **Loose coupling** — modules interact through well-defined interfaces
- **Testability by design** — every public component is independently testable
- **Value semantics** — prefer plain structs and values when ownership is simple
- **Composition over inheritance** — behaviors are composed, not inherited
- **`const` by default** — immutability where possible
- **Readability over cleverness** — code is written for humans first

Every architectural decision is made to produce a codebase that is clear,
maintainable, and a genuine portfolio piece.

---

## License

This project is a personal learning and development effort.
