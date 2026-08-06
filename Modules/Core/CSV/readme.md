# CSV Library

A C++20 CSV parsing library that reads comma-separated files via an input iterator,
supporting optional headers, user-defined delimiters, and transparent handling
of both Unix (`\n`) and Windows (`\r\n`) line endings.

## Design

The library is built around two classes:

| Class | Responsibility |
|-------|---------------|
| `CSVLineParser` | Stateless functor that splits a single line by a delimiter |
| `CSVReader` | Owns the file stream; opens lazily, strips `\r`, manages header state, provides iterators |

A nested `CSVReaderIterator` supplies **C++20 input_iterator** semantics so rows
can be consumed via range-based `for` loops or classic `begin()`/`end()` patterns.

## Types

```cpp
namespace TradingEngine::Core {

using CSVRow = std::vector<std::string>;

class CSVLineParser { ... };
class CSVReader    { ... };

} // namespace TradingEngine::Core
```

---

## Quick Start

```cpp
#include <Core/csv.hpp>
#include <iostream>

int main() {
  TradingEngine::Core::CSVReader reader{"data.csv"};

  for (const auto& row : reader) {
    for (const auto& field : row) {
      std::cout << field << " | ";
    }
    std::cout << "\n";
  }
}
```

---

## `CSVLineParser`

A stateless functor. Splits a `std::string_view` into a `CSVRow` by the given delimiter.
Trailing empty fields are preserved (`A,B,` produces three fields: `A`, `B`, `""`).

```cpp
CSVLineParser parser;
CSVRow fields = parser("a,b,c", ",");
// fields → {"a", "b", "c"}
```

---

## `CSVReader`

### Construction

```cpp
explicit CSVReader(std::filesystem::path path,
                   bool has_header = true,
                   std::string delimiter = ",");
```

- Throws `std::runtime_error` if the path does not exist on disk.
- The file is **not** opened immediately — it opens lazily on the first read.
- `has_header` controls whether the first line is treated as column names.

### Manual iteration — `get_next()`

```cpp
std::optional<CSVRow> get_next();
```

- Returns the next row, or `std::nullopt` at EOF.
- If `has_header` is true and the header has not been processed yet, the first
  call consumes and parses the header internally (it is not returned by `get_next`).

```cpp
TradingEngine::Core::CSVReader reader{"data.csv", false};
while (auto row = reader.get_next()) {
  std::cout << (*row)[0] << "\n";
}
```

### Iterator-based iteration

```cpp
reader.begin();  // resets the stream and returns a CSVReaderIterator
reader.end();    // returns std::default_sentinel
```

**Range-based for:**

```cpp
for (const auto& row : reader) {
  // row is const CSVRow&
}
```

**Classic for loop:**

```cpp
for (auto it = reader.begin(); it != reader.end(); ++it) {
  const auto& row = *it;
}
```

Calling `begin()` resets the stream (seek-to-start, clears header state)
so you can re-iterate a reader multiple times.

### Header access

```cpp
const CSVRow& get_header();       // non-const — parses header on demand
const CSVRow& get_header() const; // const     — returns already-parsed header
```

- Throws `std::logic_error` if the reader was constructed with `has_header = false`.
- The non-const overload triggers lazy header parsing if it has not been done yet.
- The const overload expects the header to already be parsed.

### Changing path or delimiter

```cpp
void set_path(std::filesystem::path path, bool has_header);
```

- Throws if the new path does not exist.
- Closes the current file and resets internal state.

```cpp
void set_delimiter(std::string delimiter);
```

- Updates the delimiter. Does not re-parse already-read data.

### Resetting

```cpp
void reset();
```

- Seeks the file back to byte 0 and clears header/processing flags.
- Also called automatically by `begin()`.

### Inspecting state

```cpp
bool get_header_processed() const;        // true after header is parsed
const std::string& get_delimiter() const; // current delimiter
bool operator!() const noexcept;          // true if the underlying stream is invalid
```

---

## `CSVReaderIterator`

Nested inside `CSVReader`. Satisfies `std::input_iterator`.

```cpp
using iterator_category = std::input_iterator_tag;
using value_type        = CSVRow;
using difference_type   = std::ptrdiff_t;
using pointer           = const CSVRow*;
using reference         = const CSVRow&;
```

| Expression | Semantics |
|-----------|-----------|
| `*it` | Returns `const CSVRow&` to the current row |
| `it->` | `const CSVRow*` access |
| `++it` / `it++` | Advances to the next row; throws `std::logic_error` if incremented past end |
| `it == it2` | Two iterators are equal iff they belong to the **same** reader instance and both are either at-end or both have a value |
| `it == std::default_sentinel` | True when the iterator is exhausted |

A default-constructed iterator acts as an end sentinel.

---

## Line ending handling

Both `\n` (LF) and `\r\n` (CRLF) line endings are supported transparently.
`parse_line` strips a trailing `\r` from each line before splitting, so
Windows-generated CSVs work unchanged on Linux/macOS.

---

## Error handling

| Scenario | Exception |
|----------|-----------|
| Path does not exist at construction | `std::runtime_error` |
| `set_path` to a non-existent file | `std::runtime_error` |
| Reading the header fails | `std::runtime_error` |
| `get_header()` when `has_header = false` | `std::logic_error` |
| Incrementing an end iterator | `std::logic_error` |

---

## CMake integration

The library is a static library aliased as `TradingEngine::Core::CSVParser`:

```cmake
target_link_libraries(my_app PRIVATE TradingEngine::Core::CSVParser)
```

Requires C++20.
