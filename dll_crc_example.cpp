// ============================================================
// Boost.DLL & Boost.CRC — combined example
// ============================================================

#include <boost/dll.hpp>
#include <boost/dll/import.hpp>
#include <boost/crc.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>

// ============================================================
// PART 1: Boost.CRC — Cyclic Redundancy Check
//
// Used for data integrity verification. Common in networking,
// file formats (PNG, ZIP, gzip), embedded protocols.
//
// boost::crc_optimal<Bits, TruncPoly, Init, RefIn, RefOut, XorOut>
// computes CRC incrementally: process_byte() / process_bytes()
// then checksum() to get result.
//
// boost::crc_basic<N> — runtime-polymorphic version (slower, flexible)
// boost::crc_optimal<N> — compile-time template version (fast)
// ============================================================

void demo_crc()
{
    std::cout << "========== Boost.CRC ==========\n" << std::endl;

    // --- CRC-32 (used in Ethernet, ZIP, PNG) ---
    {
        std::cout << "1. CRC-32 (Ethernet/ZIP/PNG standard)\n" << std::flush;
        const char* data = "The quick brown fox jumps over the lazy dog";

        boost::crc_32_type crc;
        crc.process_bytes(data, std::strlen(data));
        std::cout << "   Data:   \"" << data << "\"\n";
        std::cout << "   CRC-32: 0x" << std::hex << crc.checksum() << std::dec << '\n';

        // Verify: same data → same CRC
        boost::crc_32_type crc2;
        crc2.process_bytes(data, std::strlen(data));
        std::cout << "   Verify: " << (crc.checksum() == crc2.checksum()
                                       ? "PASS" : "FAIL") << "\n\n";
    }

    // --- CRC-16 (used in Modbus, USB) ---
    {
        std::cout << "2. CRC-16 (Modbus)\n";
        const uint8_t msg[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02};

        boost::crc_16_type crc;
        crc.process_bytes(msg, sizeof(msg));
        std::cout << "   Data:   {01 03 00 00 00 02}\n";
        std::cout << "   CRC-16: 0x" << std::hex << crc.checksum() << std::dec << "\n\n";
    }

    // --- CRC-CCITT (XMODEM, Bluetooth) ---
    {
        std::cout << "3. CRC-CCITT (XMODEM/Bluetooth)\n";
        const char* payload = "AT+CGMI\r\n";  // GSM AT command

        boost::crc_ccitt_type crc;
        crc.process_bytes(payload, std::strlen(payload));
        std::cout << "   Data:   \"" << "AT+CGMI\\r\\n" << "\"\n";
        std::cout << "   CCITT:  0x" << std::hex << crc.checksum() << std::dec << "\n\n";
    }

    // --- Custom CRC (for trading: verify order messages) ---
    {
        std::cout << "4. Custom CRC for trading messages\n";

        // CRC-8 for short message integrity (Dallas/Maxim 1-Wire)
        boost::crc_optimal<8, 0x31, 0x00, 0x00, true, true> trading_crc;

        struct OrderMsg
        {
            uint32_t order_id;
            char     symbol[8];
            double   price;
            int32_t  quantity;
            char     side;   // 'B' or 'S'
        };

        OrderMsg order{};
        order.order_id = 1293847;
        std::memcpy(order.symbol, "BTCUSDT", 8);
        order.price    = 42350.25;
        order.quantity = 100;
        order.side     = 'B';

        trading_crc.process_bytes(&order, sizeof(order));
        std::cout << "   Order #" << order.order_id
                  << " " << order.symbol
                  << " CRC-8: 0x" << std::hex
                  << (int)trading_crc.checksum() << std::dec << '\n';

        // Tamper detection: flip one bit
        order.price = 42351.25;  // wrong price!

        boost::crc_optimal<8, 0x31, 0x00, 0x00, true, true> verify_crc;
        verify_crc.process_bytes(&order, sizeof(order));
        std::cout << "   Tampered order → CRC: 0x" << std::hex
                  << (int)verify_crc.checksum()
                  << (trading_crc.checksum() == verify_crc.checksum()
                      ? " (MATCH - bad!)" : " (MISMATCH - tamper detected!)")
                  << std::dec << "\n\n";
    }

    // --- CRC with segmented processing ---
    {
        std::cout << "5. Incremental CRC (streaming data)\n";
        std::vector<double> ticks = {100.5, 101.2, 100.8, 99.7, 102.1};

        boost::crc_32_type crc;
        for (double tick : ticks)
        {
            crc.process_bytes(&tick, sizeof(tick));
            std::cout << "   + " << tick << " → intermediate CRC: 0x"
                      << std::hex << crc.checksum() << std::dec << '\n';
        }
        std::cout << "   Final: 0x" << std::hex << crc.checksum() << std::dec << '\n';
        crc.reset();  // reuse crc object for next stream
        std::cout << "   After reset: 0x" << std::hex << crc.checksum() << std::dec << "\n\n";
    }
}

// ============================================================
// PART 2: Boost.DLL — Dynamic Library Loading
//
// Load shared libraries (.so/.dll) at runtime without
// linking at compile time. Plugin architecture, hot-swapping.
//
// Key types:
//   boost::dll::shared_library    — loaded library handle
//   boost::dll::import<T>()       — import a symbol as typed fn
//   boost::dll::library_info      — inspect library metadata
//   BOOST_DLL_ALIAS               — name-mangling-safe exports
// ============================================================

void demo_dll(const std::string& plugin_path)
{
    std::cout << "========== Boost.DLL ==========\n\n";

    try
    {
        // --- Load library ---
        std::cout << "1. Loading: " << plugin_path << '\n';
        boost::dll::shared_library lib(plugin_path);
        std::cout << "   Loaded at: " << lib.location() << "\n\n";

        // --- Inspect library ---
        std::cout << "2. Library info\n";
        boost::dll::library_info info(plugin_path);
        std::cout << "   Sections: " << info.sections().size() << '\n';
        for (const auto& sec : info.sections())
            std::cout << "     " << sec << '\n';
        std::cout << "   Symbols:  " << info.symbols().size() << '\n';
        for (const auto& sym : info.symbols())
            std::cout << "     " << sym << '\n';
        std::cout << '\n';

        // --- Import functions ---
        std::cout << "3. Importing symbols\n";

        // Method A: auto type deduction via import<T>
        auto get_name = boost::dll::import_symbol<const char*()>(lib, "plugin_name");
        std::cout << "   plugin_name:  \"" << get_name() << "\"\n";

        // Method B: explicit function signature (callable wrapper)
        auto compute_sma = boost::dll::import_symbol<double(const double*, int)>(lib, "compute_sma");

        auto compute_dd = boost::dll::import_symbol<double(const double*, int)>(lib, "compute_max_drawdown");

        // --- Use imported functions ---
        std::cout << "\n4. Calling imported functions\n";
        std::vector<double> prices = {100.0, 102.0, 101.0, 105.0, 98.0, 103.0};
        std::cout << "   Prices: ";
        for (auto p : prices) std::cout << p << ' ';
        std::cout << '\n';

        double sma = compute_sma(prices.data(), (int)prices.size());
        double max_dd = compute_dd(prices.data(), (int)prices.size());
        std::cout << "   SMA(6):        " << sma << '\n';
        std::cout << "   Max Drawdown:  " << (max_dd * 100.0) << "%\n\n";

        // --- Alias symbols ---
        std::cout << "5. Aliased symbol lookup\n";
        auto name2 = boost::dll::import_alias<const char*()>(lib, "plugin_name");
        std::cout << "   Alias result:  \"" << name2() << "\"\n\n";

        // --- Symbol existence check ---
        std::cout << "6. Symbol checks\n";
        std::cout << "   has 'compute_sma': " << lib.has("compute_sma") << '\n';
        std::cout << "   has 'nonexistent': " << lib.has("nonexistent") << '\n';

        // library auto-unloads when shared_library goes out of scope
        std::cout << "\n   Library auto-unloads on scope exit.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "DLL Error: " << e.what() << '\n';
    }
}

// ============================================================
// PART 3: CRC + DLL together — verify plugin integrity
// ============================================================

void demo_crc_dll(const std::string& plugin_path)
{
    std::cout << "\n========== CRC + DLL: Plugin Integrity ==========\n\n";

    try
    {
        // Compute CRC-32 of the plugin binary itself
        std::cout << "1. Computing CRC-32 of plugin binary\n";
        boost::crc_32_type crc;
        {
            std::ifstream file(plugin_path, std::ios::binary);
            if (!file)
            {
                std::cerr << "   Cannot open " << plugin_path << '\n';
                return;
            }
            char buf[4096];
            while (file.read(buf, sizeof(buf)) || file.gcount() > 0)
                crc.process_bytes(buf, file.gcount());
        }
        uint32_t checksum = crc.checksum();
        std::cout << "   CRC-32: 0x" << std::hex << checksum << std::dec << "\n\n";

        // Verify: load library and check it has expected symbols
        // (If CRC changes, it means the library was tampered with)
        std::cout << "2. Verifying library\n";
        boost::dll::shared_library lib(plugin_path);
        bool ok = lib.has("plugin_name")
               && lib.has("compute_sma")
               && lib.has("compute_max_drawdown");
        std::cout << "   All expected symbols present: "
                  << (ok ? "YES" : "NO — library may be corrupted!") << '\n';
        std::cout << "   Integrity fingerprint: 0x" << std::hex << checksum << std::dec << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

int main(int argc, char* argv[])
{
    demo_crc();

    // Find the plugin relative to the executable
    std::string exe_dir = boost::dll::program_location().parent_path().string();
    std::string plugin  = exe_dir + "/libplugin.so";

    demo_dll(plugin);
    demo_crc_dll(plugin);
}
