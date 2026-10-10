// avalanche_strong.cpp
// Strong avalanche test — 10,000 samples × 2 runs
// AryanWebLabs

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <random>
#include <algorithm>
#include <openssl/sha.h>
#include "pegion.h"

using namespace std;
using namespace pegion;

// ============================================================
//              BIT DIFFERENCE
// ============================================================

static int bit_diff(const unsigned char* a, const unsigned char* b, size_t n) {
    int d = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char x = a[i] ^ b[i];
        while (x) { d += x & 1; x >>= 1; }
    }
    return d;
}

// ============================================================
//              AVALANCHE TEST
// ============================================================

struct Stats {
    double avg;
    double stddev;
    int min_diff;
    int max_diff;
    int samples;
};

static Stats avalanche_test(bool use_pegion, int samples) {
    vector<uint8_t> base(64, 0);
    random_device rd;
    mt19937 rng(rd());

    long long total = 0;
    long long total_sq = 0;
    int min_d = 512, max_d = 0;

    Pegion p;

    for (int s = 0; s < samples; s++) {
        for (auto& b : base) b = (uint8_t)(rng() & 0xFF);

        vector<uint8_t> flipped = base;
        int bit_pos = rng() % (base.size() * 8);
        flipped[bit_pos / 8] ^= (1 << (bit_pos % 8));

        unsigned char h1[64], h2[64];
        if (use_pegion) {
            auto d1 = p.hash512(base);
            auto d2 = p.hash512(flipped);
            memcpy(h1, d1.data(), 64);
            memcpy(h2, d2.data(), 64);
        } else {
            SHA512(base.data(), base.size(), h1);
            SHA512(flipped.data(), flipped.size(), h2);
        }

        int d = bit_diff(h1, h2, 64);
        total += d;
        total_sq += (long long)d * d;
        if (d < min_d) min_d = d;
        if (d > max_d) max_d = d;
    }

    double avg = (double)total / samples;
    double var = (double)total_sq / samples - avg * avg;
    double sd = (var > 0) ? sqrt(var) : 0;

    return {avg, sd, min_d, max_d, samples};
}

// ============================================================
//              PRINT STATS
// ============================================================

static void print_stats(const char* name, const Stats& s, int run) {
    printf("  %s [Run %d]:\n", name, run);
    printf("    Samples  : %d\n", s.samples);
    printf("    Avg      : %.4f / 512  (%.4f%%)\n",
           s.avg, s.avg * 100.0 / 512.0);
    printf("    StdDev   : %.4f bits\n", s.stddev);
    printf("    Min/Max  : %d / %d\n", s.min_diff, s.max_diff);
    printf("    Deviation: %+.4f%%\n", s.avg * 100.0 / 512.0 - 50.0);
    printf("\n");
}

// ============================================================
//              MAIN
// ============================================================

int main() {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  STRONG AVALANCHE TEST — 10,000 samples × 2 runs\n");
    printf("  (Isse pata chalega ki 50.00%% stable hai ya luck)\n");
    printf("═══════════════════════════════════════════════════════════\n\n");

    const int SAMPLES = 10000;

    // ---------- Run 1 ----------
    printf("▶ RUN 1\n\n");
    Stats sha_r1  = avalanche_test(false, SAMPLES);
    Stats peg_r1  = avalanche_test(true,  SAMPLES);
    print_stats("SHA-512",   sha_r1, 1);
    print_stats("PEGION-HA+", peg_r1, 1);

    // ---------- Run 2 ----------
    printf("▶ RUN 2\n\n");
    Stats sha_r2  = avalanche_test(false, SAMPLES);
    Stats peg_r2  = avalanche_test(true,  SAMPLES);
    print_stats("SHA-512",   sha_r2, 2);
    print_stats("PEGION-HA+", peg_r2, 2);

    // ---------- Summary ----------
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  SUMMARY\n");
    printf("═══════════════════════════════════════════════════════════\n\n");

    printf("  %-12s %12s %12s %12s\n", "Algorithm", "Run1 %", "Run2 %", "Diff");
    printf("  %-12s %12s %12s %12s\n", "---------", "------", "------", "----");
    printf("  %-12s %11.4f%% %11.4f%% %11.4f%%\n", "SHA-512",
           sha_r1.avg * 100.0 / 512.0,
           sha_r2.avg * 100.0 / 512.0,
           (sha_r2.avg - sha_r1.avg) * 100.0 / 512.0);
    printf("  %-12s %11.4f%% %11.4f%% %11.4f%%\n", "PEGION-HA+",
           peg_r1.avg * 100.0 / 512.0,
           peg_r2.avg * 100.0 / 512.0,
           (peg_r2.avg - peg_r1.avg) * 100.0 / 512.0);

    printf("\n");

    // ---------- Verdict ----------
    double sha_avg = (sha_r1.avg + sha_r2.avg) / 2.0 * 100.0 / 512.0;
    double peg_avg = (peg_r1.avg + peg_r2.avg) / 2.0 * 100.0 / 512.0;

    printf("  Average across both runs:\n");
    printf("    SHA-512    : %.4f%%\n", sha_avg);
    printf("    PEGION-HA+ : %.4f%%\n", peg_avg);
    printf("\n");

    double diff = peg_avg - sha_avg;
    if (fabs(diff) < 0.01) {
        printf("  Verdict: STATISTICALLY IDENTICAL (diff < 0.01%%)\n");
    } else if (diff > 0) {
        printf("  Verdict: PEGION-HA+ slightly BETTER (+%.4f%%)\n", diff);
    } else {
        printf("  Verdict: SHA-512 slightly better (%.4f%%)\n", diff);
    }

    printf("\n═══════════════════════════════════════════════════════════\n\n");
    return 0;
}
