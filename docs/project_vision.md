**Project Vision**

Project Name

NNFX Trading Engine

---

# Vision Statement

The goal of this project is to design and implement a production-quality automated algorithmic trading engine using modern C++20. While the initial application focuses on the No Nonsense Forex (NNFX) trading methodology using four-hour candles, the software will be designed as a modular and extensible framework capable of supporting additional trading strategies, brokers, and financial instruments in the future.

This project serves two equally important purposes:

1. To develop a reliable, maintainable, and fully automated trading system capable of generating long-term passive income through systematic trading.
2. To deepen my expertise in modern C++ software engineering by applying professional design principles, software architecture, testing methodologies, and best development practices to a real-world application.

Rather than producing a minimal proof-of-concept, the objective is to build software that resembles a commercial-grade application with a strong emphasis on maintainability, extensibility, reliability, and code quality.

---

# Project Objectives

## Functional Objectives

- Download and maintain historical market data.
- Perform accurate back-testing of trading strategies.
- Implement the complete No Nonsense Forex methodology.
- Support configurable indicators and parameters.
- Evaluate strategy performance through comprehensive statistics.
- Optimize trading parameters while minimizing over-fitting.
- Execute trades automatically through MetaTrader 5 after successful validation.
- Operate continuously on a Raspberry Pi with minimal manual intervention.

---

## Engineering Objectives

Throughout this project I aim to strengthen my understanding and practical experience with:

- Modern C++20
- Object-Oriented Design
- Generic Programming
- Software Architecture
- Design Patterns
- Test-Driven Development
- Dependency Injection
- Continuous Integration
- Performance Optimization
- Concurrent Programming (when justified)
- Networking and REST APIs
- Configuration Management
- Logging and Diagnostics
- Automated Testing

Every architectural decision should prioritize clarity, maintainability, and scalability over premature optimization.

---

# Design Philosophy

The software shall follow several core engineering principles:

- Separation of concerns.
- Single responsibility for each component.
- Loose coupling through interfaces and dependency injection.
- High cohesion within modules.
- Testability by design.
- Readability over cleverness.
- Simplicity before optimization.
- Extensibility without modification whenever practical.
- Favor composition over inheritance.
- Prefer value semantics when ownership is simple.

The project should demonstrate professional software engineering practices suitable for safety-critical and high-reliability systems.

---

# Scope

## Phase 1

Establish the project architecture, build system, development environment, configuration management, logging, testing framework, and documentation.

## Phase 2

Develop the market data subsystem capable of downloading, validating, and storing historical price data.

## Phase 3

Implement a reusable technical indicator framework.

## Phase 4

Implement the complete No Nonsense Forex trading strategy.

## Phase 5

Develop a deterministic back-testing engine capable of evaluating strategy performance.

## Phase 6

Implement parameter optimization, walk-forward analysis, and robustness testing.

## Phase 7

Integrate with MetaTrader 5 for paper trading.

## Phase 8

Deploy the application on a Raspberry Pi for continuous autonomous operation.

---

# Quality Goals

The project should satisfy the following quality attributes:

- Correctness
- Reliability
- Maintainability
- Extensibility
- Testability
- Portability
- Reproducibility
- Performance
- Documentation quality

Every public component should be documented, tested, and independently verifiable.

---

# Long-Term Vision

Although the first implementation targets the No Nonsense Forex methodology, the architecture should evolve into a reusable algorithmic trading platform capable of supporting multiple brokers, trading strategies, optimization techniques, and execution environments without requiring significant architectural changes.

Ultimately, the project should represent both a functional trading system and a showcase of modern C++ software engineering practices, demonstrating clean architecture, thoughtful design decisions, and disciplined development processes.