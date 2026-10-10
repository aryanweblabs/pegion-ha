#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <random>
#include <set>
#include <map>
#include <algorithm>
#include <openssl/sha.h>
#include "pegion.h"

using namespace std;

// ============================================================
//              HELPERS
// ============================================================

static int bit_diff(const unsigned char* a, const unsigned char* b, size_t n) {
    int d = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char x = a[i] ^ b[i];
        while (x) { d += x & 1; x >>= 1; }
    }
    return d;
}

static void pegion_hash(const vector<uint8_t>& v, unsigned char* out) {
    pegion::Pegion p;
    auto d = p.hash512(v);
    memcpy(out, d.data(), 64);
}

static void sha512_hash(const vector<uint8_t>& v, unsigned char* out) {
    SHA512(v.data(), v.size(), out);
}

// ============================================================
//              TEST 1 — STRICT AVALANCHE (1000 samples)
// ============================================================

void test_avalanche(const char* name, bool use_pegion) {
    printf("\n[TEST 1] Strict Avalanche (1000 samples)\n");

    vector<uint8_t> base(64, 0);  // 64-byte base input
    random_device rd;
    mt19937 rng(rd());

    int total_diff = 0;
    int min_diff = 512, max_diff = 0;
    const int SAMPLES = 1000;

    for (int s = 0; s < SAMPLES; s++) {
        // Random base
        for (auto& b : base) b = (uint8_t)(rng() & 0xFF);

        // Flip 1 random bit
        vector<uint8_t> flipped = base;
        int bit_pos = rng() % (base.size() * 8);
        flipped[bit_pos / 8] ^= (1 << (bit_pos % 8));

        unsigned char h1[64], h2[64];
        if (use_pegion) {
            pegion_hash(base, h1);
            pegion_hash(flipped, h2);
        } else {
            sha512_hash(base, h1);
            sha512_hash(flipped, h2);
        }

        int d = bit_diff(h1, h2, 64);
        total_diff += d;
        if (d < min_diff) min_diff = d;
        if (d > max_diff) max_diff = d;
    }

    double avg = (double)total_diff / SAMPLES;
    double pct = avg * 100.0 / 512;

    printf("  Algorithm     : %s\n", name);
    printf("  Avg bits flip : %.2f / 512  (%.2f%%)\n", avg, pct);
    printf("  Min / Max     : %d / %d\n", min_diff, max_diff);
    printf("  Ideal         : 256.00 / 512  (50.00%%)\n");
    printf("  Deviation     : %.2f%%\n", pct - 50.0);
}

// ============================================================
//              TEST 2 — BIT BALANCE
// ============================================================

void test_bit_balance(const char* name, bool use_pegion) {
    printf("\n[TEST 2] Bit Balance (10,000 hashes)\n");

    long long total_zeros = 0;
    long long total_ones = 0;

    const int SAMPLES = 10000;
    for (int i = 0; i < SAMPLES; i++) {
        vector<uint8_t> input;
        input.push_back((uint8_t)(i & 0xFF));
        input.push_back((uint8_t)((i >> 8) & 0xFF));
        input.push_back((uint8_t)((i >> 16) & 0xFF));

        unsigned char h[64];
        if (use_pegion) pegion_hash(input, h);
        else sha512_hash(input, h);

        for (int j = 0; j < 64; j++) {
            for (int b = 0; b < 8; b++) {
                if (h[j] & (1 << b)) total_ones++;
                else total_zeros++;
            }
        }
    }

    long long total = total_zeros + total_ones;
    double zero_pct = total_zeros * 100.0 / total;
    double one_pct = total_ones * 100.0 / total;

    printf("  Algorithm : %s\n", name);
    printf("  0s        : %lld  (%.4f%%)\n", total_zeros, zero_pct);
    printf("  1s        : %lld  (%.4f%%)\n", total_ones, one_pct);
    printf("  Ideal     : 50.0000%% each\n");
    printf("  Deviation : %.4f%%\n", zero_pct - 50.0);
}

// ============================================================
//              TEST 3 — BYTE DISTRIBUTION
// ============================================================

void test_byte_distribution(const char* name, bool use_pegion) {
    printf("\n[TEST 3] Byte Distribution (10,000 hashes)\n");

    map<int, long long> freq;
    for (int i = 0; i < 256; i++) freq[i] = 0;

    const int SAMPLES = 10000;
    for (int i = 0; i < SAMPLES; i++) {
        vector<uint8_t> input;
        input.push_back((uint8_t)(i & 0xFF));
        input.push_back((uint8_t)((i >> 8) & 0xFF));

        unsigned char h[64];
        if (use_pegion) pegion_hash(input, h);
        else sha512_hash(input, h);

        for (int j = 0; j < 64; j++) {
            freq[h[j]]++;
        }
    }

    // Find min/max
    long long mn = 999999999, mx = 0;
    for (auto& p : freq) {
        mn = min(mn, p.second);
        mx = max(mx, p.second);
    }

    long long total = (long long)SAMPLES * 64;
    double expected = (double)total / 256.0;

    printf("  Algorithm       : %s\n", name);
    printf("  Total bytes     : %lld\n", total);
    printf("  Expected/byte   : %.1f\n", expected);
    printf("  Min / Max freq  : %lld / %lld\n", mn, mx);
    printf("  Range deviation : %.1f%%\n", (mx - expected) * 100.0 / expected);
}

// ============================================================
//              TEST 4 — COLLISION TEST
// ============================================================

void test_collision(const char* name, bool use_pegion) {
    printf("\n[TEST 4] Collision Test (100,000 unique inputs)\n");

    set<string> seen;
    int collisions = 0;
    const int SAMPLES = 100000;

    for (int i = 0; i < SAMPLES; i++) {
        vector<uint8_t> input;
        input.push_back((uint8_t)(i & 0xFF));
        input.push_back((uint8_t)((i >> 8) & 0xFF));
        input.push_back((uint8_t)((i >> 16) & 0xFF));
        input.push_back((uint8_t)((i >> 24) & 0xFF));

        unsigned char h[64];
        if (use_pegion) pegion_hash(input, h);
        else sha512_hash(input, h);

        string s((char*)h, 32);  // first 256 bits
        if (seen.count(s)) collisions++;
        seen.insert(s);
    }

    printf("  Algorithm    : %s\n", name);
    printf("  Samples      : %d\n", SAMPLES);
    printf("  Unique       : %zu\n", seen.size());
    printf("  Collisions   : %d\n", collisions);
    printf("  Status       : %s\n",
           collisions == 0 ? "PASS ✓" : "COLLISION DETECTED ✗");
}

// ============================================================
//              TEST 5 — LENGTH SENSITIVITY
// ============================================================

void test_length_sensitivity(const char* name, bool use_pegion) {
    printf("\n[TEST 5] Length Sensitivity\n");

    // Same content, different lengths — check digest change
    vector<int> lengths = {0, 1, 2, 3, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 4096};
    vector<string> digests;

    for (int len : lengths) {
        vector<uint8_t> input(len, 'A');
        unsigned char h[64];
        if (use_pegion) pegion_hash(input, h);
        else sha512_hash(input, h);
        digests.push_back(string((char*)h, 64));
    }

    int dup = 0;
    for (size_t i = 0; i < digests.size(); i++) {
        for (size_t j = i + 1; j < digests.size(); j++) {
            if (digests[i] == digests[j]) dup++;
        }
    }

    printf("  Algorithm    : %s\n", name);
    printf("  Lengths      : %zu different\n", lengths.size());
    printf("  Unique hashes: %zu\n", digests.size() - dup);
    printf("  Duplicates   : %d\n", dup);
    printf("  Status       : %s\n",
           dup == 0 ? "PASS ✓" : "FAIL ✗");
}

// ============================================================
//              MAIN
// ============================================================

int main() {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  PEGION-HA vs SHA-512 · Cryptographic Quality Tests\n");
    printf("  (Speed excluded — pure statistical quality)\n");
    printf("═══════════════════════════════════════════════════════════\n");

    printf("\n╔════════════════════════════════════════╗\n");
    printf("║           SHA-512 (Reference)          ║\n");
    printf("╚════════════════════════════════════════╝\n");
    test_avalanche("SHA-512", false);
    test_bit_balance("SHA-512", false);
    test_byte_distribution("SHA-512", false);
    test_collision("SHA-512", false);
    test_length_sensitivity("SHA-512", false);

    printf("\n╔════════════════════════════════════════╗\n");
    printf("║         PEGION-HA (Custom)             ║\n");
    printf("╚════════════════════════════════════════╝\n");
    test_avalanche("PEGION-HA", true);
    test_bit_balance("PEGION-HA", true);
    test_byte_distribution("PEGION-HA", true);
    test_collision("PEGION-HA", true);
    test_length_sensitivity("PEGION-HA", true);

    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Quality tests complete.\n");
    printf("═══════════════════════════════════════════════════════════\n\n");

    return 0;
}
