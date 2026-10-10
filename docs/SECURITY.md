# PEGION-HA · Security Notes

*AryanWebLabs · 2026*

---

## 1. What This Document Is

This document states — **honestly and without marketing language** — what is and is not known about the security of PEGION-HA v1.0.

If you are evaluating PEGION-HA for any security-critical use, read this document first.

---

## 2. What Has Been Verified

The following properties have been **empirically tested** on PEGION-HA v1.0.

| Property | Result | Method |
|---|---|---|
| Determinism | ✅ Confirmed | Two runs → identical digest |
| Avalanche (1-bit input change) | 50.02% ± 0.05% | 10,000 samples × 2 runs |
| Bit balance | ±0.0078% | 50,000 hashes |
| Byte distribution | 2.49% range deviation | 50,000 hashes |
| Collision resistance | 0 collisions | 100,000 unique inputs (256-bit truncation) |
| Length sensitivity | 0 duplicates | 14 input lengths |
| NIST SP 800-22 | 15/15 tests pass | 100 × 1M-bit sequences |
| Bit-perfect round-trip | ✅ Confirmed | 100× encrypt/decrypt audit |

**Interpretation:** PEGION-HA produces output that passes standard statistical tests and matches SHA-512 in avalanche and byte-distribution behaviour.

---

## 3. What Has NOT Been Verified

The following properties have **not** been tested. Their absence is not a claim of weakness — it is a statement of unknown status.

| Property | Status |
|---|---|
| Collision resistance (full 512-bit) | Not tested (2²⁵⁶ attempts infeasible) |
| Preimage resistance | Not analysed |
| Second-preimage resistance | Not analysed |
| Length-extension resistance | Not tested |
| Differential cryptanalysis | Not performed |
| Linear cryptanalysis | Not performed |
| Algebraic cryptanalysis | Not performed |
| Side-channel resistance (timing) | Not analysed |
| Side-channel resistance (power/cache) | Not analysed |
| Third-party review | None |
| Formal security reduction | None |

**Interpretation:** A lack of published attacks is not evidence of security. PEGION-HA has not been subjected to the years of adversarial analysis that established algorithms have.

---

## 4. Comparison With Established Algorithms

For reference, here is how PEGION-HA compares with commonly deployed hashes along the axes that matter for security decisions.

| Aspect | SHA-2 | SHA-3 | BLAKE3 | PEGION-HA |
|---|---|---|---|---|
| Structure | Merkle-Damgård | Sponge | Tree + ChaCha | Custom |
| Age | 24 years | 10 years | 5 years | < 1 year |
| Public cryptanalysis | Extensive | Extensive | Moderate | None |
| Side-channel analysis | Extensive | Extensive | Moderate | None |
| NIST standardisation | FIPS 180-4 | FIPS 202 | None | None |
| Production deployments | Billions | Millions | Many | Zero |
| Statistical test pass (NIST SP 800-22) | Yes | Yes | Yes | **Yes** |

The last row is the only row where PEGION-HA matches the established algorithms.

---

## 5. Threat Model

### 5.1 Attacker Capabilities Considered

PEGION-HA is designed to resist:

- An attacker who knows the algorithm and observes the digest.
- An attacker who observes many (input, digest) pairs.
- An attacker who attempts to find two inputs with the same digest.

### 5.2 Attacker Capabilities Not Considered

PEGION-HA does **not** claim resistance to:

- An attacker with significant cryptanalytic expertise and time.
- An attacker with the ability to mount side-channel attacks on the implementation.
- A nation-state adversary.

---

## 6. Recommendations

### 6.1 For Learning and Research

PEGION-HA is suitable for:
- Studying the design and analysis of hash functions.
- Teaching examples in a cryptography course.
- Reproducible research experiments.

### 6.2 For Personal Use

PEGION-HA is **acceptable** for:
- Encrypting personal files that are not of high value.
- Educational projects.
- Experiments where loss of data is recoverable.

### 6.3 For Production

PEGION-HA is **not suitable** for:
- Any security-critical system.
- Data whose compromise would cause harm.
- Long-term storage.
- Regulatory compliance contexts.

For production use, prefer:
- **Hashing:** SHA-256, SHA-512, BLAKE2b, BLAKE3, SHA-3.
- **Encryption:** AES-256-GCM, ChaCha20-Poly1305.
- **Password hashing:** Argon2id, scrypt, PBKDF2.

---

## 7. Design Notes on Specific Properties

### 7.1 Length-Extension Resistance

SHA-2 is vulnerable to length-extension because of its Merkle-Damgård structure. PEGION-HA uses a non-sequential input distribution (PIGEONHOLE-PRIME) and a Davies-Meyer-style feed-forward. Whether these properties prevent length extension has not been formally analysed.

**Status:** Unknown.

### 7.2 Keyed Mode

PEGION-HA's keyed mode absorbs the key as a preliminary block, followed by the padded message. This is a standard NMAC-like construction, but unlike HMAC it has not been proven secure under the assumption that the underlying compression function is a PRF.

**Status:** Reasonable construction; not formally analysed.

### 7.3 Determinism and Nonce Handling

The hash itself is deterministic. When used within the RBFA-Vault stream cipher, a per-file random salt (32 bytes) and nonce (16 bytes) ensure the keystream is different for every file.

**Status:** Correct, but relies on the OS RNG for salt/nonce generation.

---

## 8. Version History

| Version | Changes | Security Impact |
|---|---|---|
| v1.0 | Initial release | Baseline |
| v1.0+ | Bit-balance fix (squeeze) | No security change |
| v1.0+ | Byte-distribution fix (pigeonhole) | No security change |

---

## 9. Contact

For security-related questions or to report issues:

*AryanWebLabs*

---

*End of Security Notes*
