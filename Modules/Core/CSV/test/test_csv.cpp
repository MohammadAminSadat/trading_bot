#include <Core/csv.hpp>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace TEC = TradingEngine::Core;

namespace {

struct TempFile {
  std::filesystem::path path;

  explicit TempFile(const std::string &content) {
    static std::atomic<unsigned> counter{0};
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    auto name = "csv_test_" + std::to_string(now) + "_" + std::to_string(::getpid()) + "_" +
                std::to_string(++counter) + ".csv";
    path = std::filesystem::temp_directory_path() / name;
    std::ofstream ofs(path);
    ofs << content;
  }

  ~TempFile() { std::filesystem::remove(path); }
};

} // namespace

// =============================================================================
// CSVLineParser tests
// =============================================================================

TEST(CSVLineParser, SplitsByComma) {
  TEC::CSVLineParser parser;
  auto row = parser("a,b,c", ",");
  EXPECT_EQ(row.size(), 3u);
  EXPECT_EQ(row[0], "a");
  EXPECT_EQ(row[1], "b");
  EXPECT_EQ(row[2], "c");
}

TEST(CSVLineParser, SplitsByCustomDelimiter) {
  TEC::CSVLineParser parser;
  auto row = parser("a|b|c", "|");
  EXPECT_EQ(row.size(), 3u);
  EXPECT_EQ(row[0], "a");
  EXPECT_EQ(row[1], "b");
  EXPECT_EQ(row[2], "c");
}

TEST(CSVLineParser, SplitsByTab) {
  TEC::CSVLineParser parser;
  auto row = parser("x\ty\tz", "\t");
  EXPECT_EQ(row.size(), 3u);
  EXPECT_EQ(row[0], "x");
  EXPECT_EQ(row[1], "y");
  EXPECT_EQ(row[2], "z");
}

TEST(CSVLineParser, TrailingEmptyField) {
  TEC::CSVLineParser parser;
  auto row = parser("A,B,", ",");
  ASSERT_EQ(row.size(), 3u);
  EXPECT_EQ(row[0], "A");
  EXPECT_EQ(row[1], "B");
  EXPECT_EQ(row[2], "");
}

TEST(CSVLineParser, ConsecutiveDelimiters) {
  TEC::CSVLineParser parser;
  auto row = parser("A,,B", ",");
  ASSERT_EQ(row.size(), 3u);
  EXPECT_EQ(row[0], "A");
  EXPECT_EQ(row[1], "");
  EXPECT_EQ(row[2], "B");
}

TEST(CSVLineParser, LeadingDelimiter) {
  TEC::CSVLineParser parser;
  auto row = parser(",B,C", ",");
  ASSERT_EQ(row.size(), 3u);
  EXPECT_EQ(row[0], "");
  EXPECT_EQ(row[1], "B");
  EXPECT_EQ(row[2], "C");
}

TEST(CSVLineParser, EmptyLine) {
  TEC::CSVLineParser parser;
  auto row = parser("", ",");
  ASSERT_EQ(row.size(), 1u);
  EXPECT_EQ(row[0], "");
}

TEST(CSVLineParser, SingleFieldNoDelimiter) {
  TEC::CSVLineParser parser;
  auto row = parser("hello", ",");
  ASSERT_EQ(row.size(), 1u);
  EXPECT_EQ(row[0], "hello");
}

TEST(CSVLineParser, MultiCharacterDelimiter) {
  TEC::CSVLineParser parser;
  auto row = parser("a::b::c", "::");
  ASSERT_EQ(row.size(), 3u);
  EXPECT_EQ(row[0], "a");
  EXPECT_EQ(row[1], "b");
  EXPECT_EQ(row[2], "c");
}

// =============================================================================
// CSVReader construction tests
// =============================================================================

TEST(CSVReader, ThrowsOnNonExistentPath) {
  EXPECT_THROW(TEC::CSVReader("/nonexistent/path/file.csv"), std::runtime_error);
}

TEST(CSVReader, ConstructingWithoutHeaderArgumentTreatsFirstLineAsHeader) {
  TempFile f("a,b,c\n1,2,3\n");
  TEC::CSVReader reader{f.path};
  auto row = reader.get_next();
  ASSERT_TRUE(row.has_value());
  EXPECT_EQ((*row)[0], "1");
  EXPECT_EQ((*row)[1], "2");
  EXPECT_EQ((*row)[2], "3");
}

TEST(CSVReader, DefaultDelimiterIsComma) {
  TempFile f("a,b\n");
  TEC::CSVReader reader{f.path};
  EXPECT_EQ(reader.get_delimiter(), ",");
}

// =============================================================================
// get_next() tests — no header
// =============================================================================

TEST(CSVReader, GetNextWithoutHeader) {
  TempFile f("1,2,3\n4,5,6\n");
  TEC::CSVReader reader{f.path, false};

  auto row1 = reader.get_next();
  ASSERT_TRUE(row1.has_value());
  EXPECT_EQ(row1->size(), 3u);
  EXPECT_EQ((*row1)[0], "1");
  EXPECT_EQ((*row1)[1], "2");
  EXPECT_EQ((*row1)[2], "3");

  auto row2 = reader.get_next();
  ASSERT_TRUE(row2.has_value());
  EXPECT_EQ((*row2)[0], "4");

  auto eof = reader.get_next();
  EXPECT_FALSE(eof.has_value());
}

TEST(CSVReader, GetNextWithHeader) {
  TempFile f("h1,h2,h3\n10,20,30\n40,50,60\n");
  TEC::CSVReader reader{f.path, true};

  auto row1 = reader.get_next();
  ASSERT_TRUE(row1.has_value());
  EXPECT_EQ((*row1)[0], "10");
  EXPECT_EQ((*row1)[1], "20");
  EXPECT_EQ((*row1)[2], "30");
}

// =============================================================================
// get_next() with CRLF line endings
// =============================================================================

TEST(CSVReader, GetNextWithCRLF) {
  TempFile f("a,b,c\r\nx,y,z\r\n");
  TEC::CSVReader reader{f.path, false};

  auto row = reader.get_next();
  ASSERT_TRUE(row.has_value());
  EXPECT_EQ(row->size(), 3u);
  EXPECT_EQ((*row)[0], "a");
  EXPECT_EQ((*row)[1], "b");
  EXPECT_EQ((*row)[2], "c");
  EXPECT_NE((*row)[2].back(), '\r'); // \r is stripped
}

// =============================================================================
// header tests
// =============================================================================

TEST(CSVReader, GetHeaderAfterFirstNext) {
  TempFile f("col1,col2,col3\na1,a2,a3\n");
  TEC::CSVReader reader{f.path, true};

  EXPECT_FALSE(reader.get_header_processed());

  auto header = reader.get_header();
  ASSERT_EQ(header.size(), 3u);
  EXPECT_EQ(header[0], "col1");
  EXPECT_EQ(header[1], "col2");
  EXPECT_EQ(header[2], "col3");
  EXPECT_TRUE(reader.get_header_processed());
}

TEST(CSVReader, GetHeaderParsesOnDemand) {
  TempFile f("X,Y,Z\ndata,data,data\n");
  TEC::CSVReader reader{f.path, true};

  auto h = reader.get_header();
  EXPECT_EQ(h[0], "X");
  EXPECT_EQ(h[1], "Y");
  EXPECT_EQ(h[2], "Z");
  EXPECT_TRUE(reader.get_header_processed());
}

TEST(CSVReader, GetHeaderConstAfterParsed) {
  TempFile f("A,B\ndata,data\n");
  TEC::CSVReader reader{f.path, true};
  reader.get_next(); // triggers header parsing

  const auto &creader = reader;
  auto h = creader.get_header();
  EXPECT_EQ(h[0], "A");
  EXPECT_EQ(h[1], "B");
}

TEST(CSVReader, GetHeaderThrowsWhenNoHeader) {
  TempFile f("data\n");
  TEC::CSVReader reader{f.path, false};
  EXPECT_THROW(reader.get_header(), std::logic_error);
}

TEST(CSVReader, GetHeaderProcessedFlag) {
  TempFile f("h1,h2\ndata\n");
  TEC::CSVReader reader{f.path, true};
  EXPECT_FALSE(reader.get_header_processed());
  reader.get_header();
  EXPECT_TRUE(reader.get_header_processed());
}

// =============================================================================
// set_path tests
// =============================================================================

TEST(CSVReader, SetPathThrowsOnNonExistent) {
  TempFile f("a,b\n");
  TEC::CSVReader reader{f.path};
  EXPECT_THROW(reader.set_path("/no/such/file.csv", false), std::runtime_error);
}

TEST(CSVReader, SetPathSwitchesFile) {
  TempFile f1("old1,old2\n");
  TempFile f2("h1,h2,h3\ndata1,data2,data3\n");
  TEC::CSVReader reader{f1.path, false};

  reader.set_path(f2.path, true);
  auto row = reader.get_next();
  ASSERT_TRUE(row.has_value());
  EXPECT_EQ(row->size(), 3u);
  EXPECT_EQ((*row)[0], "data1");
}

// =============================================================================
// set_delimiter tests
// =============================================================================

TEST(CSVReader, SetDelimiterChangesParsing) {
  TempFile f("a|b|c\nd|e|f\n");
  TEC::CSVReader reader{f.path, true, ","}; // default comma
  reader.set_delimiter("|");

  EXPECT_EQ(reader.get_delimiter(), "|");
  // header parsed with | after first get_next
  auto header = reader.get_header();
  ASSERT_EQ(header.size(), 3u);
  EXPECT_EQ(header[0], "a");
  EXPECT_EQ(header[1], "b");
  EXPECT_EQ(header[2], "c");
}

// =============================================================================
// reset / re-iteration tests
// =============================================================================

TEST(CSVReader, ResetRewindsStream) {
  TempFile f("x1,x2\n");
  TEC::CSVReader reader{f.path, false};

  auto first = reader.get_next();
  ASSERT_TRUE(first.has_value());
  EXPECT_EQ((*first)[0], "x1");

  reader.reset();

  auto again = reader.get_next();
  ASSERT_TRUE(again.has_value());
  EXPECT_EQ((*again)[0], "x1");
}

TEST(CSVReader, BeginResetsAndReiterates) {
  TempFile f("hdr\nr1,r2\nr3,r4\n");
  TEC::CSVReader reader{f.path, true};

  // First pass
  int c1{0};
  for (const auto &row : reader) {
    (void)row;
    ++c1;
  }
  EXPECT_EQ(c1, 2);

  // Second pass — begin() calls reset()
  int c2{0};
  for (const auto &row : reader) {
    (void)row;
    ++c2;
  }
  EXPECT_EQ(c2, 2);
}

// =============================================================================
// Iterator tests — range-for
// =============================================================================

TEST(CSVReader, RangeForIteration) {
  TempFile f("h\n1,2\n3,4\n5,6\n");
  TEC::CSVReader reader{f.path, true};

  int count{0};
  for (const auto &row : reader) {
    EXPECT_EQ(row.size(), 2u);
    ++count;
  }
  EXPECT_EQ(count, 3);
}

TEST(CSVReader, RangeForEmptyFile) {
  TempFile f("");
  TEC::CSVReader reader{f.path, false};
  int count{0};
  for (const auto &row : reader) {
    (void)row;
    ++count;
  }
  EXPECT_EQ(count, 0);
}

// =============================================================================
// Iterator tests — classic for loop
// =============================================================================

TEST(CSVReader, ClassicForLoop) {
  TempFile f("h1,h2\n1,2\n3,4\n");
  TEC::CSVReader reader{f.path, true};

  int count{0};
  for (auto it = reader.begin(); it != reader.end(); ++it) {
    ++count;
  }
  EXPECT_EQ(count, 2);
}

TEST(CSVReader, ClassicForLoopWithPostIncrement) {
  TempFile f("h\na\nb\n");
  TEC::CSVReader reader{f.path, true};

  int count{0};
  for (auto it = reader.begin(); it != reader.end(); it++) {
    ++count;
  }
  EXPECT_EQ(count, 2);
}

// =============================================================================
// Iterator comparison tests
// =============================================================================

TEST(CSVReader, IteratorEqualsEnd) {
  TempFile f("h\nx\n");
  TEC::CSVReader reader{f.path, true};

  auto it = reader.begin();
  EXPECT_NE(it, reader.end());
  ++it;
  EXPECT_EQ(it, reader.end());
}

TEST(CSVReader, IteratorCompareDefaultSentinel) {
  TempFile f("h\na\n");
  TEC::CSVReader reader{f.path, true};

  auto it = reader.begin();
  std::default_sentinel_t sent;
  EXPECT_NE(it, sent);
  EXPECT_NE(sent, it);
  ++it;
  EXPECT_EQ(it, sent);
  EXPECT_EQ(sent, it);
}

TEST(CSVReader, IteratorsFromSameReaderCompare) {
  TempFile f("h\nr1\nr2\n");
  TEC::CSVReader reader{f.path, true};

  auto it = reader.begin();
  auto end = reader.end();

  // a copy is equal
  auto copy{it};
  EXPECT_EQ(it, copy);

  // advance the copy — shares the stream, so it reads the next row
  ++copy;

  // copy now has a different value than it
  // (both have a value → operator== returns true; this is expected
  // input-iterator semantics — equality is position-unaware)
  ++it;

  // advance both to end
  ++it;
  ++copy;
  EXPECT_EQ(it, end);
  EXPECT_EQ(copy, end);
  EXPECT_EQ(it, copy);

  // default-constructed iterators compare equal
  TEC::CSVReader::Iterator e1;
  TEC::CSVReader::Iterator e2;
  EXPECT_EQ(e1, e2);
  EXPECT_EQ(e1, end);
}

// =============================================================================
// Iterator dereference tests
// =============================================================================

TEST(CSVReader, IteratorArrowOperator) {
  TempFile f("h\n1,2,3\n");
  TEC::CSVReader reader{f.path, true};

  auto it = reader.begin();
  EXPECT_EQ(it->size(), 3u);
  EXPECT_EQ((*it)[0], "1");
}

TEST(Iterator, IncrementEndIteratorThrows) {
  auto end = TEC::CSVReader::Iterator{};
  EXPECT_THROW(++end, std::logic_error);
}

// =============================================================================
// Edge cases
// =============================================================================

TEST(CSVReader, TrailingCommaPreservesEmptyField) {
  TempFile f("h1,h2,\n");
  TEC::CSVReader reader{f.path, true};

  auto h = reader.get_header();
  ASSERT_EQ(h.size(), 3u);
  EXPECT_EQ(h[0], "h1");
  EXPECT_EQ(h[1], "h2");
  EXPECT_EQ(h[2], "");
}

TEST(CSVReader, SingleLineFileWithHeader) {
  TempFile f("only,header\n");
  TEC::CSVReader reader{f.path, true};

  auto h = reader.get_header();
  EXPECT_EQ(h[0], "only");
  EXPECT_EQ(h[1], "header");

  auto next = reader.get_next();
  EXPECT_FALSE(next.has_value());
}

TEST(CSVReader, SingleLineFileNoHeader) {
  TempFile f("single\n");
  TEC::CSVReader reader{f.path, false};

  auto row = reader.get_next();
  ASSERT_TRUE(row.has_value());
  EXPECT_EQ((*row)[0], "single");

  auto eof = reader.get_next();
  EXPECT_FALSE(eof.has_value());
}

TEST(CSVReader, OperatorNotOnInvalidStream) {
  TempFile f("ok\n");
  TEC::CSVReader reader{f.path, false};
  EXPECT_FALSE(!reader);

  // read to EOF — stream is still valid (eofbit, not failbit)
  reader.get_next();
  EXPECT_FALSE(!reader);
}

TEST(CSVReader, CRLFHeaderIsClean) {
  TempFile f("a,b\r\n1,2\r\n");
  TEC::CSVReader reader{f.path, true};

  auto h = reader.get_header();
  ASSERT_EQ(h.size(), 2u);
  EXPECT_EQ(h[0], "a");
  EXPECT_EQ(h[1], "b");
  EXPECT_NE(h[1].back(), '\r');
}

// =============================================================================
// Documented limitations: quoted CSV (RFC 4180) is not supported
// =============================================================================

TEST(CSVLineParser, QuotedFieldContainingDelimiterIsSplit) {
  TEC::CSVLineParser parser;
  // No RFC 4180 support: the quote does not protect the comma, so the line
  // is split at every delimiter. This documents the current behavior.
  auto row = parser("\"a,b\",c", ",");
  ASSERT_EQ(row.size(), 3u);
  EXPECT_EQ(row[0], "\"a");
  EXPECT_EQ(row[1], "b\"");
  EXPECT_EQ(row[2], "c");
}

TEST(CSVLineParser, EscapedQuotesAreNotUnescaped) {
  TEC::CSVLineParser parser;
  auto row = parser("\"a\"\"b\"", ",");
  ASSERT_EQ(row.size(), 1u);
  EXPECT_EQ(row[0], "\"a\"\"b\"");
}

TEST(CSVReader, QuotedFieldContainingDelimiterIsSplitInFile) {
  TempFile f("\"a,b\",c\n");
  TEC::CSVReader reader{f.path, false};

  auto row = reader.get_next();
  ASSERT_TRUE(row.has_value());
  ASSERT_EQ(row->size(), 3u);
  EXPECT_EQ((*row)[0], "\"a");
  EXPECT_EQ((*row)[1], "b\"");
  EXPECT_EQ((*row)[2], "c");
}

// =============================================================================
// Stream-state tests
// =============================================================================

TEST(CSVReader, ResetAfterEofClearsStreamState) {
  TempFile f("h\n1,2\n3,4\n");
  TEC::CSVReader reader{f.path, true};

  EXPECT_TRUE(reader.get_next().has_value());
  EXPECT_TRUE(reader.get_next().has_value());
  EXPECT_FALSE(reader.get_next().has_value()); // EOF

  reader.reset();

  auto row = reader.get_next();
  ASSERT_TRUE(row.has_value());
  EXPECT_EQ((*row)[0], "1");
}

TEST(CSVReader, SetPathAfterEofClearsStreamState) {
  TempFile f1("h\n1,2\n3,4\n");
  TempFile f2("h\na,b\nc,d\n");
  TEC::CSVReader reader{f1.path, true};

  EXPECT_TRUE(reader.get_next().has_value());
  EXPECT_TRUE(reader.get_next().has_value());
  EXPECT_FALSE(reader.get_next().has_value()); // EOF on f1

  reader.set_path(f2.path, true);

  auto row = reader.get_next();
  ASSERT_TRUE(row.has_value());
  EXPECT_EQ((*row)[0], "a");
}

// =============================================================================
// Iterator invalidation / behavior after set_path
// =============================================================================

TEST(CSVReader, IteratorContinuesOnNewPathAfterSetPath) {
  TempFile f1("h\nold1\nold2\n");
  TempFile f2("h\nnew1\nnew2\n");
  TEC::CSVReader reader{f1.path, true};

  auto it = reader.begin();
  EXPECT_EQ((*it)[0], "old1");

  // set_path closes the current stream; an existing iterator stays bound to
  // the reader and continues reading from the newly-set file on ++.
  reader.set_path(f2.path, true);

  ++it;
  ASSERT_EQ((*it)[0], "new1");

  ++it;
  ASSERT_EQ((*it)[0], "new2");

  ++it;
  EXPECT_EQ(it, reader.end());
}

// =============================================================================
// Move semantics tests (CSVReader is move-only)
// =============================================================================

TEST(CSVReader, MoveConstructionKeepsReading) {
  TempFile f("h\n1,2\n3,4\n");
  TEC::CSVReader r1{f.path, true};
  TEC::CSVReader r2{std::move(r1)};

  auto row = r2.get_next();
  ASSERT_TRUE(row.has_value());
  EXPECT_EQ((*row)[0], "1");
  EXPECT_EQ((*row)[1], "2");
}

TEST(CSVReader, MoveConstructionPreservesHeader) {
  TempFile f("c1,c2\nx,y\n");
  TEC::CSVReader r1{f.path, true};
  TEC::CSVReader r2{std::move(r1)};

  auto h = r2.get_header();
  ASSERT_EQ(h.size(), 2u);
  EXPECT_EQ(h[0], "c1");
  EXPECT_EQ(h[1], "c2");
}

TEST(CSVReader, MoveAssignmentKeepsReading) {
  TempFile f1("h\n1,2\n3,4\n");
  TempFile f2("other\n");
  TEC::CSVReader r1{f1.path, true};
  TEC::CSVReader r2{f2.path, false};

  r2 = std::move(r1);

  auto row = r2.get_next();
  ASSERT_TRUE(row.has_value());
  EXPECT_EQ((*row)[0], "1");
}

// =============================================================================
// Header + iteration combinations
// =============================================================================

TEST(CSVReader, GetHeaderThenRangeForSkipsHeader) {
  TempFile f("h1,h2\n1,2\n3,4\n");
  TEC::CSVReader reader{f.path, true};

  auto h = reader.get_header();
  EXPECT_EQ(h[0], "h1");

  int count{0};
  for (const auto &row : reader) {
    ASSERT_EQ(row.size(), 2u);
    ++count;
  }
  EXPECT_EQ(count, 2);
}

TEST(CSVReader, GetNextThenResetThenRangeFor) {
  TempFile f("h\n1\n2\n");
  TEC::CSVReader reader{f.path, true};

  ASSERT_TRUE(reader.get_next().has_value());
  reader.reset();

  int count{0};
  for (const auto &row : reader) {
    ++count;
  }
  EXPECT_EQ(count, 2);
}

TEST(CSVReader, GetHeaderThenClassicLoopSkipsHeader) {
  TempFile f("h1,h2\n1,2\n3,4\n");
  TEC::CSVReader reader{f.path, true};

  reader.get_header();

  int count{0};
  for (auto it = reader.begin(); it != reader.end(); ++it) {
    ASSERT_EQ(it->size(), 2u);
    ++count;
  }
  EXPECT_EQ(count, 2);
}
