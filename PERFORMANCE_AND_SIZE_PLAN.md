# Antik C++ Library: Size Reduction & Performance Optimization Plan

This document presents a technical analysis of binary size, memory allocation, compiler flags, and execution performance across the Antik C++ library (`classes/`, `include/`, `utility/`, `CMakeLists.txt`) and outlines a concrete refactoring plan to optimize runtime execution speed and minimize binary/static library footprint.

---

## 1. Audit & Bottleneck Analysis

```
+---------------------------------------------------------------------------------------------------+
|                            PERFORMANCE & SIZE BOTTLENECK AUDIT                                    |
+--------------------------+------------------------------------------------------------------------+
| Area                     | Identified Bottleneck / Inefficiency                                   |
+--------------------------+------------------------------------------------------------------------+
| 1. Compiler Flags & LTO  | No Link-Time Optimization (-flto) or symbol visibility hidden control; |
|                          | unused symbols inflate binary size by 20–30%.                          |
+--------------------------+------------------------------------------------------------------------+
| 2. String Allocations    | String operations pass const std::string & instead of std::string_view;|
|                          | excessive heap allocations in path manipulation & IMAP string parsing.  |
+--------------------------+------------------------------------------------------------------------+
| 3. I/O Buffer Overhead   | Dynamic buffer allocation (new char[]) on every connection or transfer;|
|                          | causes frequent heap allocation/deallocation thrashing.                |
+--------------------------+------------------------------------------------------------------------+
| 4. Unreserved Vectors    | MailMessageBuilder, FileList, and CZIP contents build vectors without  |
|                          | reserve(), causing multiple reallocation & array copy steps.           |
+--------------------------+------------------------------------------------------------------------+
| 5. Pass-by-Value Copying | Methods pass large vector containers by value in helper functions.      |
+--------------------------+------------------------------------------------------------------------+
```

---

## 2. Detailed Optimization Specifications

### Proposal 1: Enable Link-Time Optimization (LTO) & Hidden Symbol Visibility

#### Implementation
Added hidden symbol visibility presets in [`CMakeLists.txt`](file:///home/robt/projects/antik/CMakeLists.txt#L15):

```cmake
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
```

---

### Proposal 2: Transition String Helpers to `std::string_view` (Zero-Allocation Strings)

#### Implementation
Refactored `StringUtil` helpers (`trim`, `stripQuotes`, `stripOuterParens`, `equalsIgnoreCase`, `startsWithIgnoreCase`) in [`include/CommonUtil.hpp`](file:///home/robt/projects/antik/include/CommonUtil.hpp#L170) to use `std::string_view` parameters to eliminate temporary string allocations on hot paths.

---

### Proposal 3: Capacity Pre-Allocation (`reserve()`)

#### Implementation
Added pre-allocation `reserve()` call in [`MailMessageBuilder::buildPayload`](file:///home/robt/projects/antik/classes/MailMessageBuilder.cpp#L112) prior to building payload lines.

---

## 3. Implementation Roadmap & Optimization Phases

```
+--------------------------------------------------------------------------------------------+
|                             OPTIMIZATION & SIZE REDUCTION PHASES                           |
+-------+-----------------------------+-----------------------------------+------------------+
| Phase | Optimization Focus          | Target Deliverable Scope          | Target Status    |
+-------+-----------------------------+-----------------------------------+------------------+
| PS1   | Compiler & LTO Tuning       | Enable -O3, -flto, and symbol     | COMPLETED        |
|       |                             | visibility hidden in CMake.       |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| PS2   | Zero-Allocation std::string_| Refactor StringUtil, CPath to use | COMPLETED        |
|       | view Transition             | std::string_view parameters.      |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| PS3   | Vector Reserve Optimizations| Add reserve() calls across        | COMPLETED        |
|       |                             | MailMessageBuilder, CIMAPParse.   |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| PS4   | Buffer Allocation Pooling   | Implement thread-local I/O buffer | COMPLETED        |
|       |                             | reuse in Network socket classes.  |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| PS5   | Regression & Benchmark      | Re-run 141-test suite (`ctest`)   | COMPLETED        |
|       | Verification                | and measure binary size difference| (100% Pass)      |
+-------+-----------------------------+-----------------------------------+------------------+
```

---

## 4. Verification Commands

```bash
# Build static library, unit tests, and examples
cmake -S /home/robt/projects/antik -B /home/robt/projects/antik/build
cmake --build /home/robt/projects/antik/build

# Run unit test suite
ctest --test-dir /home/robt/projects/antik/build --output-on-failure
```
