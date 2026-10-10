// main.cpp
// PEGION-HA command-line tool
// AryanWebLabs
//
// Usage:
//   pegion hash    <file>                    → PEGION-512 digest
//   pegion hash256 <file>                    → PEGION-256 digest
//   pegion encrypt <input> <output> <pass>   → encrypt to .phx
//   pegion decrypt <input> <output> <pass>   → decrypt .phx
//   pegion verify  <input> <expected-hex>    → verify hash

#include "pegion.h"
#include "pegion_stream.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

using namespace pegion;

// ============================================================
//                    HELPERS
// ============================================================

static std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open: " + path);
    return std::vector<uint8_t>(
        (std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>());
}

static void print_banner() {
    std::cout << "\n";
    std::cout << "  ◈  PEGION-HA  •  Pigeonhole Lattice Hash\n";
    std::cout << "     AryanWebLabs · v1.0.0\n\n";
}

static void print_usage(const char* prog) {
    std::cout << "Usage:\n\n";
    std::cout << "  " << prog << " hash    <file>\n";
    std::cout << "      → Print PEGION-512 digest\n\n";
    std::cout << "  " << prog << " hash256 <file>\n";
    std::cout << "      → Print PEGION-256 digest\n\n";
    std::cout << "  " << prog << " encrypt <input> <output> <password>\n";
    std::cout << "      → Encrypt file to .phx container\n\n";
    std::cout << "  " << prog << " decrypt <input> <output> <password>\n";
    std::cout << "      → Decrypt .phx file\n\n";
    std::cout << "  " << prog << " verify  <file> <expected-hex>\n";
    std::cout << "      → Compare digest against expected value\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << prog << " hash    test.txt\n";
    std::cout << "  " << prog << " encrypt test.txt test.phx mypass\n";
    std::cout << "  " << prog << " decrypt test.phx restored.txt mypass\n\n";
}

// ============================================================
//                    COMMANDS
// ============================================================

static int cmd_hash(const std::string& path, bool use_256) {
    auto data = read_file(path);
    Pegion p;

    if (use_256) {
        auto d = p.hash256(data);
        std::cout << "File:       " << path << "\n";
        std::cout << "Size:       " << data.size() << " bytes\n";
        std::cout << "PEGION-256: " << Pegion::to_hex(d) << "\n";
    } else {
        auto d = p.hash512(data);
        std::cout << "File:       " << path << "\n";
        std::cout << "Size:       " << data.size() << " bytes\n";
        std::cout << "PEGION-512: " << Pegion::to_hex(d) << "\n";
    }
    return 0;
}

static int cmd_encrypt(const std::string& in,
                       const std::string& out,
                       const std::string& pass) {
    std::cout << "Encrypting: " << in << "\n";
    std::cout << "Output:     " << out << "\n";
    std::cout << "Deriving key (100,000 rounds)... " << std::flush;

    auto t0 = std::chrono::high_resolution_clock::now();

    PegionStream stream;
    stream.encrypt_file(in, out, pass);

    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "done.\n";
    std::cout << "Time:       " << (int)ms << " ms\n";
    std::cout << "Status:     ✓ Encrypted successfully\n";
    return 0;
}

static int cmd_decrypt(const std::string& in,
                       const std::string& out,
                       const std::string& pass) {
    std::cout << "Decrypting: " << in << "\n";
    std::cout << "Output:     " << out << "\n";
    std::cout << "Deriving key (100,000 rounds)... " << std::flush;

    auto t0 = std::chrono::high_resolution_clock::now();

    PegionStream stream;
    stream.decrypt_file(in, out, pass);

    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "done.\n";
    std::cout << "Time:       " << (int)ms << " ms\n";
    std::cout << "Status:     ✓ Decrypted (bit-perfect)\n";
    return 0;
}

static int cmd_verify(const std::string& path, const std::string& expected) {
    auto data = read_file(path);
    Pegion p;
    auto d = p.hash512(data);
    std::string actual = Pegion::to_hex(d);

    std::cout << "File:     " << path << "\n";
    std::cout << "Expected: " << expected << "\n";
    std::cout << "Actual:   " << actual << "\n\n";

    if (actual == expected) {
        std::cout << "Status:   ✓ MATCH\n";
        return 0;
    } else {
        std::cout << "Status:   ✗ MISMATCH\n";
        return 2;
    }
}

// ============================================================
//                    MAIN
// ============================================================

int main(int argc, char** argv) {
    print_banner();

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string cmd = argv[1];

    try {
        if (cmd == "hash" && argc == 3) {
            return cmd_hash(argv[2], false);
        }
        else if (cmd == "hash256" && argc == 3) {
            return cmd_hash(argv[2], true);
        }
        else if (cmd == "encrypt" && argc == 5) {
            return cmd_encrypt(argv[2], argv[3], argv[4]);
        }
        else if (cmd == "decrypt" && argc == 5) {
            return cmd_decrypt(argv[2], argv[3], argv[4]);
        }
        else if (cmd == "verify" && argc == 4) {
            return cmd_verify(argv[2], argv[3]);
        }
        else {
            print_usage(argv[0]);
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "\n✗ Error: " << e.what() << "\n";
        return 1;
    }
}
