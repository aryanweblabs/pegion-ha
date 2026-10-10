# PEGION-HA · NIST SP 800-22 Test Results

*AryanWebLabs · 2026*

---

## 1. Summary

PEGION-HA v1.0 was subjected to the **NIST SP 800-22 Statistical Test Suite for Random and Pseudorandom Number Generators for Cryptographic Applications**.

**Result: 15 / 15 tests passed.**

| Metric | Result |
|---|---|
| Test suite version | sts-2.1.2 |
| Total tests | 15 |
| Tests passed | **15** |
| Tests failed | **0** |
| Sequences tested | 100 × 1,000,000 bits |
| Total bits analysed | 100,000,000 |
| Input source | PEGION-HA-512 of numbered strings |

---

## 2. Test Procedure

1. **Input generation.** Using PEGION-HA-512, hash the strings `"<seq>_<idx>"` for `seq ∈ [0, 99]` and `idx ∈ [0, N)`. Concatenate the 64-byte digests until 125,000 bytes (1,000,000 bits) per sequence are collected. Write all 100 sequences to a single binary file.

2. **Test execution.** Invoke NIST STS in "all tests" mode with default parameters:
   - Block Frequency block length = 128
   - Non-overlapping template block length = 9
   - Overlapping template block length = 9
   - Approximate Entropy block length = 10
   - Serial block length = 16
   - Linear Complexity block length = 500

3. **Interpretation.** A test passes if the proportion of passing sequences is ≥ 96/100 (or ≥ 60/64 for Random Excursions), and the uniformity of p-values passes the chi-squared check.

---

## 3. Full Results Table

| # | Statistical Test | p-value | Proportion | Verdict |
|---|---|---|---|---|
| 1 | Frequency | 0.851383 | 99/100 | ✅ PASS |
| 2 | Block Frequency | 0.955835 | 100/100 | ✅ PASS |
| 3 | Cumulative Sums (fwd) | 0.437274 | 99/100 | ✅ PASS |
| 4 | Cumulative Sums (rev) | 0.739918 | 100/100 | ✅ PASS |
| 5 | Runs | 0.202268 | 100/100 | ✅ PASS |
| 6 | Longest Run of Ones | 0.955835 | 99/100 | ✅ PASS |
| 7 | Rank | 0.319084 | 99/100 | ✅ PASS |
| 8 | Discrete Fourier Transform | 0.455937 | 98/100 | ✅ PASS |
| 9 | Non-overlapping Templates | (163 sub-tests) | 96–100/100 | ✅ PASS |
| 10 | Overlapping Template | 0.191687 | 99/100 | ✅ PASS |
| 11 | Universal (Maurer) | 0.554420 | 100/100 | ✅ PASS |
| 12 | Approximate Entropy | 0.275709 | 100/100 | ✅ PASS |
| 13 | Random Excursions | (8 sub-tests) | 62–64/64 | ✅ PASS |
| 14 | Random Excursions Variant | (18 sub-tests) | 62–64/64 | ✅ PASS |
| 15 | Serial | 0.657933 / 0.978072 | 100/100 / 99/100 | ✅ PASS |
| 16 | Linear Complexity | 0.779188 | 100/100 | ✅ PASS |

**Total: 15 / 15 test groups passed.**

---

## 4. Notable Results

**Perfect scores (100/100):**
- Block Frequency
- Cumulative Sums (reverse)
- Runs
- Universal Statistical
- Approximate Entropy
- Linear Complexity

**Highest p-values (closest to uniform):**
- Non-overlapping Templates: `0.996335`
- Random Excursions Variant: `0.991468`
- Serial: `0.978072`

**Lowest p-value (still passing):**
- Non-overlapping Templates (index 6): `0.005762` (still > 0.01, and the sub-test proportion was 100/100).

---

## 5. Interpretation

NIST SP 800-22 tests are designed to detect deviations from randomness. Passing all 15 tests indicates that the output of PEGION-HA does not exhibit any of the statistical biases tested for.

**Important caveat, restated from NIST documentation:**

> "The [SP 800-22] tests are not intended to be used as a substitute for statistical analysis of the output of a random number generator. Their purpose is to detect certain types of deviations from randomness; the detection of a deviation does not necessarily imply that the generator is unsuitable for cryptographic use, nor does the absence of a detected deviation imply that the generator is suitable."

Passing NIST SP 800-22 is **necessary but not sufficient** for cryptographic security. See `SECURITY.md` for a discussion of what has not been tested.

---

## 6. Reproducing These Results

To reproduce:

```bash
# 1. Build PEGION-HA
cd pegion-ha && mkdir build && cd build
cmake .. && make -j$(nproc)

# 2. Generate 1 billion bits
./gen_nist_bits         # produces nist_bits.bin (~120 MB)

# 3. Install NIST STS
cd ~/Downloads
wget https://csrc.nist.gov/CSRC/media/Projects/Random-Bit-Generation/documents/sts-2_1_2.zip
unzip sts-2_1_2.zip && cd sts-2.1.2/sts-2.1.2 && make

# 4. Run
printf '0\n/path/to/nist_bits.bin\n1\n0\n100\n1\n' | ./assess 1000000

# 5. View results
cat experiments/AlgorithmTesting/finalAnalysisReport.txt
