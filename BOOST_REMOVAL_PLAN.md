# Antik C++ Library: Boost Dependency Removal & Native Abstractions Plan

This document presents a technical audit of Boost dependencies across all 16 example executables in `examples/` and outlines a concrete refactoring plan to eliminate Boost reliance in favor of native C++17 features (`std::filesystem`) and a new lightweight Antik command-line option parser (`CProgramOptions`).

---

## 1. Boost Usage Audit across Examples

```
+---------------------------------------------------------------------------------------------------+
|                                 BOOST DEPENDENCY AUDIT & GAPS                                     |
+--------------------------+------------------------------------------------------------------------+
| Boost Feature            | Example Files Impacted & Proposed Replacement                          |
+--------------------------+------------------------------------------------------------------------+
| boost::program_options   | Used across all 16 examples (option parsing & config files).           |
|                          | Replaced with new native Antik class: CProgramOptions.                 |
+--------------------------+------------------------------------------------------------------------+
| boost::filesystem        | Path checking, file size, directory creation in examples.              |
|                          | Replaced with standard C++17 std::filesystem and CPath.                 |
+--------------------------+------------------------------------------------------------------------+
| boost::algorithm::string | String trimming, splitting, and case conversions.                      |
|                          | Replaced with Antik StringUtil and std::string_view helpers.           |
+--------------------------+------------------------------------------------------------------------+
```

---

## 2. Technical Specifications for Native Antik Abstractions

### Component 1: Command-Line Option Parser (`CProgramOptions`)

#### Implementation
Created `CProgramOptions` class in [`include/CProgramOptions.hpp`](file:///home/robt/projects/antik/include/CProgramOptions.hpp) and [`classes/CProgramOptions.cpp`](file:///home/robt/projects/antik/classes/CProgramOptions.cpp):

```cpp
namespace Antik::Util {

class CProgramOptions {
public:
    struct Option {
        std::string flagShort;
        std::string flagLong;
        std::string description;
        bool hasValue{false};
        std::string value;
        std::string defaultValue;
        bool isSet{false};
    };

    explicit CProgramOptions(const std::string &programDescription = "");

    void addOption(const std::string &flagShort, const std::string &flagLong, const std::string &description, bool hasValue = false, const std::string &defaultValue = "");
    bool parse(int argc, char **argv);
    bool loadConfigFile(const std::string &configFilePath);

    bool count(const std::string &name) const;
    std::string get(const std::string &name) const;
    int getInt(const std::string &name) const;
    void printHelp(std::ostream &os = std::cout) const;
};

} // namespace Antik::Util
```

---

## 3. Implementation Roadmap & Phases

```
+--------------------------------------------------------------------------------------------+
|                               BOOST REMOVAL PHASES                                         |
+-------+-----------------------------+-----------------------------------+------------------+
| Phase | Category                    | Target Deliverable Scope          | Target Status    |
+-------+-----------------------------+-----------------------------------+------------------+
| BR1   | Antik Options Abstraction   | Create CProgramOptions class in   | COMPLETED        |
|       |                             | CProgramOptions.hpp/cpp.          |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| BR2   | Refactor Example Options    | Replace boost::program_options    | COMPLETED        |
|       |                             | across all 16 examples.           |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| BR3   | Refactor Example Filesystem | Replace boost::filesystem with    | COMPLETED        |
|       |                             | std::filesystem and CPath.        |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| BR4   | CMake Dependency Removal    | Remove Boost dependency from      | COMPLETED        |
|       |                             | CMakeLists.txt.                   |                  |
+-------+-----------------------------+-----------------------------------+------------------+
| BR5   | Regression & Verification   | Re-build static library, examples | COMPLETED        |
|       |                             | and run 141-test suite (ctest).   | (100% Pass)      |
+-------+-----------------------------+-----------------------------------+------------------+
```

---

## 4. Verification Commands

```bash
# Build static library, unit tests, and examples without Boost
cmake -S /home/robt/projects/antik -B /home/robt/projects/antik/build
cmake --build /home/robt/projects/antik/build

# Run unit test suite
ctest --test-dir /home/robt/projects/antik/build --output-on-failure
```
