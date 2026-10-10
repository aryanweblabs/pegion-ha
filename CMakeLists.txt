# CMakeLists.txt
# PEGION-HA — Pigeonhole Lattice Hash + Stream Cipher
# AryanWebLabs · v1.1.0

cmake_minimum_required(VERSION 3.15)
project(pegion_ha VERSION 1.1.0 LANGUAGES CXX)

# ---------- C++ standard ----------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- Default build type ----------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------- Optimization flags ----------
# -O3          : full optimization
# -march=native: use CPU's full instruction set (AVX2, AVX-512, etc.)
# -mtune=native: tune for this specific CPU
# -funroll-loops: unroll small loops
# -flto        : link-time optimization (cross-file inlining)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# ---------- Optional OpenSSL acceleration ----------
option(PEGION_USE_OPENSSL "Use OpenSSL for SHA (4-5x faster)" OFF)

# ---------- Threading (for potential parallel features) ----------
find_package(Threads REQUIRED)

# ============================================================
#                    CORE LIBRARY
# ============================================================
add_library(pegion_core STATIC
    pegion_opt.cpp          # optimized implementation (2x faster)
    pegion_stream.cpp       # stream cipher (uses hash)
)

target_include_directories(pegion_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pegion_core PUBLIC Threads::Threads)

# ---------- Optional OpenSSL linking ----------
if(PEGION_USE_OPENSSL)
    find_package(OpenSSL REQUIRED)
    message(STATUS "PEGION: OpenSSL enabled → SHA acceleration ON")
    target_compile_definitions(pegion_core PRIVATE PEGION_USE_OPENSSL)
    target_link_libraries(pegion_core PUBLIC OpenSSL::Crypto)
else()
    message(STATUS "PEGION: OpenSSL disabled → using custom SHA")
endif()

# ============================================================
#                    EXECUTABLES
# ============================================================

# ---------- Self-test ----------
add_executable(pegion_test pegion_test.cpp)
target_link_libraries(pegion_test PRIVATE pegion_core)

# ---------- CLI tool ----------
add_executable(pegion-cli main.cpp)
target_link_libraries(pegion-cli PRIVATE pegion_core)

# ============================================================
#                    INSTALL
# ============================================================
install(TARGETS pegion_core ARCHIVE DESTINATION lib)
install(TARGETS pegion-cli  RUNTIME DESTINATION bin)
install(FILES
    pegion.h
    pegion_stream.h
    DESTINATION include
)

# ============================================================
#                    INFO
# ============================================================
message(STATUS "")
message(STATUS "PEGION-HA build configuration:")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "  OpenSSL:        ${PEGION_USE_OPENSSL}")
message(STATUS "  Compiler flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "")
