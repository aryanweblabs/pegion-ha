# PEGION-HA · Design Specification

**Pigeonhole Lattice Hash Algorithm — v1.0**

*AryanWebLabs · 2026*

---

## 1. Overview

PEGION-HA is a cryptographic hash function built on a novel construction called the **Pigeonhole Lattice**. It produces a 512-bit digest from arbitrary-length input, with optional keyed operation for MAC use cases.

| Property | Value |
|---|---|
| Digest size | 512 bits (64 bytes) |
| Internal state | 1024 bits (16 × 64-bit words) |
| Block size | 128 bytes |
| Rounds per block | 32 (LATTICE-STEP cycles) |
| Structure | Custom (non-Merkle-Damgård, non-sponge) |
| Keyed mode | Yes (built-in MAC) |

PEGION-HA does not derive from SHA-1, SHA-2, SHA-3, BLAKE, BLAKE2, BLAKE3, Keccak, Groestl, Skein, JH, or any other established design.

---

## 2. Notation

| Symbol | Meaning |
|---|---|
| `W[i]` | The `i`-th 64-bit word of the state (i ∈ [0, 15]) |
| `M` | Input message (arbitrary byte length) |
| `⊕` | Bitwise XOR |
| `+` | Addition modulo 2⁶⁴ |
| `ROTL(x, n)` | Left rotation of x by n bits |
| `ROTR(x, n)` | Right rotation of x by n bits |
| `‖` | Concatenation |

All arithmetic is performed on 64-bit unsigned integers with wraparound.

---

## 3. High-Level Structure
