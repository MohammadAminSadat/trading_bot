# Trading Bot - Software Engineering Project Plan

> A software engineering-first approach to building a trading bot in C++, designed as a learning journey to master modern C++ development practices.

## Overview

This project treats the trading bot as a software engineering project first and a trading bot second. The goal is to become a better C++ developer, with every phase teaching new engineering skills rather than just adding features.

---

## Phase 1 — Foundation & Architecture

**Goal:** Build a production-quality project skeleton with clean architecture, development tools, and coding standards before writing any trading logic.

---

### Milestone 1 — Project Planning

#### Task 1.1 Define Requirements

- [ ] Write a project vision document
- [ ] Define functional requirements
- [ ] Define non-functional requirements
- [ ] Define future extensions (MT5, optimization, dashboard)

**Deliverables:**
- `docs/requirements.md`

---

#### Task 1.2 Choose Technology Stack

Decide on:
- [ ] C++20
- [ ] CMake
- [ ] Git
- [ ] clang-format
- [ ] clang-tidy
- [ ] Catch2
- [ ] fmt
- [ ] spdlog
- [ ] nlohmann/json
- [ ] cpr or Boost.Beast (HTTP client)
- [ ] SQLite

**Deliverables:**
- `docs/technology.md`

---

### Milestone 2 — Software Architecture

#### Task 2.1 High-Level Architecture

Design:
- [ ] Components
- [ ] Dependencies
- [ ] Data flow

Draw UML:
- [ ] Component Diagram
- [ ] Package Diagram

---

#### Task 2.2 Domain Model

Design:
- [ ] Candle
- [ ] Order
- [ ] Position
- [ ] Trade
- [ ] Indicator
- [ ] Strategy

Create UML class diagrams.

---

#### Task 2.3 Project Structure

```bash
TradingBot/
├── src/
├── include/
├── tests/
├── docs/
├── config/
├── scripts/
├── data/
├── build/
└── third_party/
```

---

### Milestone 3 — Development Environment

#### Task 3.1 Repository

- [ ] Create Git repository
- [ ] Create .gitignore
- [ ] Protect main branch
- [ ] Create development branch

---

#### Task 3.2 Build System

Create `CMakeLists.txt`

Requirements:
- [ ] C++20
- [ ] Warning flags
- [ ] Debug build
- [ ] Release build

---

#### Task 3.3 Code Quality

Configure:
- [ ] clang-format
- [ ] clang-tidy
- [ ] cppcheck

---

#### Task 3.4 CI (Optional)

GitHub Actions:
- [ ] Build
- [ ] Run tests
- [ ] Static analysis

---

### Milestone 4 — Core Library

Create namespace:
```cpp
namespace trading
{
}
```
---

Task 4.1 Common Types

Create:

- Price
- Timestamp
- Volume
- Symbol
- TimeFrame

---

Task 4.2 Enums

- [ ] OrderType
- [ ] TradeDirection
- [ ] OrderStatus
- [ ] IndicatorType
- [ ] SignalType

---

Task 4.3 Utilities

Implement:
- [ ] UUID generator
- [ ] Time utilities
- [ ] String utilities
- [ ] File utilities

---

### Milestone 5 — Configuration System

Design config.json

Example:
```json
{
  "symbol": "EURUSD",
  "timeframe": "4H",
  "risk": 0.01
}
```
Tasks:
- [ ] Read JSON
- [ ] Validate fields
- [ ] Default values
- [ ] Error handling

---

### Milestone 6 — Logging System

Create Logger

Capabilities:
- [ ] Console logging
- [ ] File logging
- [ ] Log levels:
    - [ ] INFO
    - [ ] DEBUG
    - [ ] WARNING
    - [ ] ERROR

---

### Milestone 7 — Error Handling

Design policy for handling:
- Network failure
- Invalid candle
- Missing file
- Invalid configuration

Guideline: Avoid throwing exceptions everywhere. Use `std::expected` (or an equivalent) where appropriate for recoverable errors, and reserve exceptions for truly exceptional situations.

---

### Milestone 8 — Testing
- [ ] Install GoogleTest
- [ ] Write first tests:
    - [ ] Config Parser
    - [ ] Logger
    - [ ] Utilities
    - [ ] Time conversion

Target: 100% passing tests

---

### Milestone 9 — Documentation

Write README.md

Include:
- Build instructions
- Dependencies
- Architecture
- Folder structure
- Coding guidelines

---

### Design Decisions Log
>Recommended Addition: Since this is intended to be a learning journey, keep a docs/design-decisions/ folder from the very beginning. For every major architectural choice, write a one-page design note.

Examples of decisions to document:
- Why use dependency injection?
- Why value types for Candle?
- Why `std::unique_ptr` here?
- Why choose spdlog over other logging libraries?
- Why use `std::expected` for error handling?

This habit mirrors professional engineering practices and will make the project an excellent portfolio piece because you'll be able to explain not just what you built, but why you built it that way.
---
### Notes
- Every phase should teach a new set of engineering skills
- Focus on clean architecture and production-quality code
- Treat this as a learning journey in modern C++ development
- Document all design decisions for portfolio presentation