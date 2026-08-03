#include <boost/flyweight.hpp>
#include <boost/flyweight/key_value.hpp>
#include <boost/flyweight/no_tracking.hpp>
#include <boost/flyweight/set_factory.hpp>
#include <iostream>
#include <string>
#include <vector>

// ============================================================
// Boost.Flyweight — "Flyweight" design pattern.
//
// Problem:    Storing many duplicate objects (e.g. strings)
//             wastes memory.
// Solution:   Flyweight stores identical values only ONCE.
//             Copies share the same internal representation.
//             Compare by pointer, not by value.
//
// Flyweight<T> acts like T but uses internal factory to
// deduplicate. Default factory = boost::hashed_factory
// (hash set). You can customize: tags, tracking, locking.
// ============================================================

void demo_default()
{
    std::cout << "=== 1. Default flyweight (hash-based dedup) ===\n\n";

    // Each "AAPL" refers to the same internal string object
    boost::flyweight<std::string> a1("AAPL");
    boost::flyweight<std::string> a2("AAPL");
    boost::flyweight<std::string> b("GOOGL");

    std::cout << "a1 = " << a1 << '\n';
    std::cout << "a2 = " << a2 << '\n';
    std::cout << "b  = " << b  << '\n';

    // Same value => same address (pointer comparison works!)
    std::cout << "&a1.get() == &a2.get(): "
              << (&a1.get() == &a2.get() ? "TRUE (shared!)" : "false")
              << '\n';
    std::cout << "&a1.get() == &b.get():  "
              << (&a1.get() == &b.get()  ? "TRUE" : "false (different)")
              << '\n';

    // Cheap copy — just increments refcount
    auto a3 = a1;
    std::cout << "&a1.get() == &a3.get(): "
              << (&a1.get() == &a3.get() ? "TRUE" : "false") << '\n';

    // Equality still works normally
    std::cout << "a1 == a2: " << (a1 == a2 ? "true" : "false") << '\n';
    std::cout << "a1 == b:  " << (a1 == b  ? "true" : "false") << '\n';
    std::cout << '\n';
}

// ============================================================
// Tagged flyweights — separate factories per tag.
// Different tags = different internal pools.
// Same tag = same pool (dedup within same tag).
// ============================================================

struct symbol_tag {};
struct exchange_tag {};

void demo_tags()
{
    std::cout << "=== 2. Tagged flyweights (separate pools) ===\n\n";

    using symbol   = boost::flyweight<std::string, boost::flyweights::tag<symbol_tag>>;
    using exchange = boost::flyweight<std::string, boost::flyweights::tag<exchange_tag>>;

    symbol   s1("NASDAQ");
    exchange e1("NASDAQ");  // Same string, different pool

    // They are in different pools, so addresses differ
    std::cout << "symbol   'NASDAQ': " << s1 << '\n';
    std::cout << "exchange 'NASDAQ': " << e1 << '\n';

    // But equality still works across tags
    std::cout << "s1.get() == e1.get(): "
              << (s1.get() == e1.get() ? "true (value equal)"
                                       : "false") << '\n';
    std::cout << '\n';
}

// ============================================================
// Customization: ordered factory + no tracking.
//
// set_factory     → uses std::set (ordered), O(log n) lookup
// no_tracking     → no refcount, values never deleted
//                   (good for static data that lives forever)
// ============================================================

struct currency_tag {};

using currency = boost::flyweight<
    std::string,
    boost::flyweights::tag<currency_tag>,
    boost::flyweights::set_factory<>,          // ordered (std::set)
    boost::flyweights::no_tracking             // never freed
>;

void demo_custom()
{
    std::cout << "=== 3. Custom: set_factory + no_tracking ===\n\n";

    currency c1("USD");
    currency c2("EUR");
    currency c3("USD");    // shares with c1
    currency c4("JPY");
    currency c5("EUR");    // shares with c2

    std::cout << "c1 = " << c1 << '\n';
    std::cout << "c2 = " << c2 << '\n';
    std::cout << "c3 = " << c3 << " (same as c1? "
              << (&c1.get() == &c3.get() ? "YES" : "NO") << ")\n";
    std::cout << "c5 = " << c5 << " (same as c2? "
              << (&c2.get() == &c5.get() ? "YES" : "NO") << ")\n";
    std::cout << '\n';
}

// ============================================================
// Practical: OrderBook with thousands of repeating symbols.
//
// Without flyweight: 1M orders × "BTCUSDT" (8 bytes) = 8 MB
//                     for JUST the symbol string.
// With flyweight:    1M orders × 1 pointer           = 8 MB
//                    (on 64-bit) but strings stored once.
//
// Real saving: 1M orders, 100 unique symbols:
//     without: 1M * sizeof(string) ≈ 32 MB (SSO buffer)
//     with:    1M * sizeof(ptr) + 100 strings ≈ 8 MB
// ============================================================

struct Order
{
    boost::flyweight<std::string> symbol;
    double price;
    double quantity;
    bool   is_buy;
};

void demo_orderbook()
{
    std::cout << "=== 4. Practical: OrderBook ===\n\n";

    std::vector<Order> orders;

    // 1000 orders, only 5 unique symbols
    const char* symbols[] = {"BTCUSDT", "ETHUSDT", "SOLUSDT", "DOGEUSDT", "ADAUSDT"};

    for (int i = 0; i < 1000; ++i)
    {
        Order o;
        o.symbol   = symbols[i % 5];
        o.price    = 40000.0 + (i % 1000) * 0.5;
        o.quantity = (i % 10 + 1) * 0.01;
        o.is_buy   = (i % 3 != 0);
        orders.push_back(std::move(o));
    }

    // All orders with same symbol share the same string
    std::cout << "Orders: " << orders.size() << '\n';
    std::cout << "Unique symbols: " << std::size(symbols) << '\n';

    bool shared = (&orders[0].symbol.get() == &orders[5].symbol.get());
    std::cout << "orders[0].symbol == orders[5].symbol: "
              << (shared ? "SAME pointer (deduped!)" : "different")
              << '\n';

    // Iterate
    for (int i = 0; i < 6; ++i)
    {
        const auto& o = orders[i];
        std::cout << "  " << o.symbol << " | "
                  << (o.is_buy ? "BUY " : "SELL") << " | "
                  << o.quantity << " @ " << o.price << '\n';
    }

    std::cout << '\n';
}

// ============================================================
// key_value: flyweight where the "key" (what's deduplicated)
//            is different from the "value" (what you use).
//
// Example: dedup by ticker ID, but store full Ticker struct.
// ============================================================

struct Ticker
{
    int    id;
    const char* name;
    const char* sector;
};

struct extract_key
{
    using result_type = int;
    const result_type& operator()(const Ticker& t) const
    {
        // key_value requires returning a const reference — cache the value
        key_ = t.id;
        return key_;
    }
private:
    mutable result_type key_;
};

void demo_key_value()
{
    std::cout << "=== 5. key_value (dedup by ID, store full struct) ===\n\n";

    using ticker_fw = boost::flyweight<
        boost::flyweights::key_value<int, Ticker, extract_key>
    >;

    ticker_fw t1(Ticker{1, "Apple Inc.", "Technology"});
    ticker_fw t2(Ticker{1, "Apple Inc.", "Technology"}); // same id=1 → dedup
    ticker_fw t3(Ticker{2, "JPMorgan",   "Finance"});

    // t1 and t2 share the same Ticker object
    std::cout << "t1.name = " << t1.get().name
              << " (" << t1.get().sector << ")\n";
    std::cout << "t2.name = " << t2.get().name
              << " (" << t2.get().sector << ")\n";
    std::cout << "t3.name = " << t3.get().name
              << " (" << t3.get().sector << ")\n";

    std::cout << "&t1.get() == &t2.get(): "
              << (&t1.get() == &t2.get() ? "TRUE (deduped!)" : "false")
              << '\n';
    std::cout << '\n';
}

int main()
{
    demo_default();
    demo_tags();
    demo_custom();
    demo_orderbook();
    demo_key_value();
}
