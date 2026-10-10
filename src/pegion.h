// pegion.h
// PEGION-HA — Pigeonhole Lattice Hash Function
// AryanWebLabs · Version 1.1.0
//
// A custom cryptographic hash algorithm.
// Structure: Pigeonhole Lattice
// State: 1024-bit (16 × 64-bit words)
// Rounds: 32 (LATTICE-STEP cycles)
// Output: 512-bit (default) / 384 / 256
//
// Not derived from SHA, Blake, Keccak, Groestl, or Skein.

#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace pegion {

// ============================================================
//                    CONFIGURATION
// ============================================================
constexpr int    WORD_COUNT    = 16;      // 16 words
constexpr int    WORD_BITS     = 64;      // 64-bit words
constexpr int    STATE_BITS    = WORD_COUNT * WORD_BITS;  // 1024
constexpr int    CYCLES        = 32;      // LATTICE-STEP rounds
constexpr int    BLOCK_BYTES   = 128;     // input block = 128 bytes
constexpr int    DIGEST_512    = 64;      // bytes
constexpr int    DIGEST_384    = 48;
constexpr int    DIGEST_256    = 32;

// ============================================================
//                       TYPES
// ============================================================
using Word       = uint64_t;
using State      = std::array<Word, WORD_COUNT>;
using Digest512  = std::array<uint8_t, DIGEST_512>;
using Digest384  = std::array<uint8_t, DIGEST_384>;
using Digest256  = std::array<uint8_t, DIGEST_256>;

// ============================================================
//                       API
// ============================================================
class Pegion {
public:
    Pegion();
    explicit Pegion(const std::vector<uint8_t>& key);

    // Primary hashing
    Digest512 hash512(const std::vector<uint8_t>& data);
    Digest512 hash512(const std::string& text);

    // Truncated variants
    Digest384 hash384(const std::vector<uint8_t>& data);
    Digest256 hash256(const std::vector<uint8_t>& data);

    // Keyed variant (MAC-like)
    Digest512 mac512(const std::vector<uint8_t>& data,
                     const std::vector<uint8_t>& key);

    // Hex helpers
    static std::string to_hex(const Digest512& d);
    static std::string to_hex(const Digest384& d);
    static std::string to_hex(const Digest256& d);

private:
    // ---------- Core ----------
    void init_state();
    void absorb_block(const uint8_t* block);
    void lattice_step(int cycle);
    void pigeonhole_prime(const uint8_t* block);
    void feed_forward(const State& saved);
    void feed_forward_from(const Word* saved);        // optimized helper
    void pigeonhole_fold(std::vector<uint8_t>& data);
    Digest512 squeeze(const State& state, uint64_t msg_len);
    void final_mix(State& s, int passes);

    // ---------- Helpers ----------
    static inline Word rotl(Word x, int n) {
        return (x << n) | (x >> (64 - n));
    }
    static inline Word rotr(Word x, int n) {
        return (x >> n) | (x << (64 - n));
    }

    State             m_state;
    std::array<Word, CYCLES> m_constants;
    bool              m_keyed;
};

// Utility
std::vector<uint8_t> to_bytes(const std::string& s);

} // namespace pegion
