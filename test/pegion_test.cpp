// pegion_test.cpp
// PEGION-HA self-test suite
// AryanWebLabs

#include "pegion.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>

using namespace pegion;

// ============================================================
//                    HELPERS
// ============================================================

static int bit_diff(const uint8_t* a, const uint8_t* b, size_t n) {
    int diff = 0;
    for (size_t i = 0; i < n; i++) {
        uint8_t x = a[i] ^ b[i];
        while (x) { diff += x & 1; x >>= 1; }
    }
    return diff;
}

static void print_sep(const std::string& title) {
    std::cout << "\n──────────────────────────────────────────────\n";
    std::cout << "  " << title << "\n";
    std::cout << "──────────────────────────────────────────────\n";
}

// ============================================================
//                    TEST 1 — Determinism
// ============================================================

void test_determinism() {
    print_sep("TEST 1 — Determinism");

    Pegion p1, p2;
    auto h1 = p1.hash512(std::string("hello world"));
    auto h2 = p2.hash512(std::string("hello world"));

    std::cout << "PEGION-512(\"hello world\") =\n  "
              << Pegion::to_hex(h1) << "\n\n";
    std::cout << "Status: " << (h1 == h2 ? "PASS ✓" : "FAIL ✗") << "\n";
}

// ============================================================
//                    TEST 2 — Avalanche
// ============================================================

void test_avalanche() {
    print_sep("TEST 2 — Avalanche Effect");

    Pegion p;
    auto ha = p.hash512(std::string("hello"));
    auto hb = p.hash512(std::string("hellp"));  // 1 char change

    int diff = bit_diff(ha.data(), hb.data(), DIGEST_512);
    double pct = diff * 100.0 / 512.0;

    std::cout << "Input A: \"hello\"\n";
    std::cout << "Input B: \"hellp\"  (1 char diff)\n\n";
    std::cout << "A = " << Pegion::to_hex(ha) << "\n\n";
    std::cout << "B = " << Pegion::to_hex(hb) << "\n\n";
    std::cout << "Bit diff: " << diff << "/512  ("
              << std::fixed << std::setprecision(1) << pct << "%)\n";
    std::cout << "Expected: ~50% (256 bits)\n\n";
    std::cout << "Status: "
              << ((diff > 200 && diff < 312) ? "PASS ✓" : "WEAK ✗") << "\n";
}

// ============================================================
//                    TEST 3 — Empty input
// ============================================================

void test_empty() {
    print_sep("TEST 3 — Empty Input");

    Pegion p;
    auto h = p.hash512(std::string(""));
    std::cout << "PEGION-512(\"\") =\n  "
              << Pegion::to_hex(h) << "\n";
}

// ============================================================
//                    TEST 4 — Multi-block
// ============================================================

void test_long() {
    print_sep("TEST 4 — Multi-Block (10000 bytes)");

    std::string long_text(10000, 'A');
    Pegion p;
    auto h = p.hash512(long_text);
    std::cout << "PEGION-512(10k 'A') =\n  "
              << Pegion::to_hex(h) << "\n";
}

// ============================================================
//                    TEST 5 — MAC
// ============================================================

void test_mac() {
    print_sep("TEST 5 — Keyed MAC");

    Pegion p;
    std::vector<uint8_t> k1 = {'k', 'e', 'y', '1'};
    std::vector<uint8_t> k2 = {'k', 'e', 'y', '2'};
    auto msg = to_bytes("secret message");

    auto m1 = p.mac512(msg, k1);
    auto m2 = p.mac512(msg, k2);

    std::cout << "MAC(key1, msg) = " << Pegion::to_hex(m1) << "\n\n";
    std::cout << "MAC(key2, msg) = " << Pegion::to_hex(m2) << "\n\n";
    std::cout << "Differ: " << (m1 != m2 ? "PASS ✓" : "FAIL ✗") << "\n";
}

// ============================================================
//                    TEST 6 — Variants
// ============================================================

void test_variants() {
    print_sep("TEST 6 — Output Variants");

    Pegion p;
    auto data = to_bytes("hello");

    std::cout << "PEGION-512: " << Pegion::to_hex(p.hash512(data)) << "\n\n";
    std::cout << "PEGION-384: " << Pegion::to_hex(p.hash384(data)) << "\n\n";
    std::cout << "PEGION-256: " << Pegion::to_hex(p.hash256(data)) << "\n";
}

// ============================================================
//                    TEST 7 — Performance
// ============================================================

void test_performance() {
    print_sep("TEST 7 — Performance");

    std::string msg(1024 * 1024, 'x');   // 1 MB

    Pegion p;
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10; i++) {
        p.hash512(msg);
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double throughput = (10.0 * 1.0) / (ms / 1000.0);  // MB/s

    std::cout << "Hashed 10 MB in " << std::fixed << std::setprecision(2)
              << ms << " ms\n";
    std::cout << "Throughput: ~" << std::setprecision(1)
              << throughput << " MB/s\n";
}

// ============================================================
//                    MAIN
// ============================================================

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║      PEGION-HA · Self Test Suite v1.0        ║\n";
    std::cout << "║      Pigeonhole Lattice Hash                 ║\n";
    std::cout << "║      AryanWebLabs                            ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";

    try {
        test_determinism();
        test_avalanche();
        test_empty();
        test_long();
        test_mac();
        test_variants();
        test_performance();

        std::cout << "\n──────────────────────────────────────────────\n";
        std::cout << "  All tests complete.\n";
        std::cout << "──────────────────────────────────────────────\n\n";
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
