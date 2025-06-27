# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ArffFiles is a header-only C++ library for reading ARFF (Attribute-Relation File Format) files and converting them into STL vectors. The library handles both numeric and categorical features, automatically factorizing categorical attributes.

## Build System

This project uses CMake with Conan for package management:
- **CMake**: Primary build system (requires CMake 3.20+)
- **Conan**: Package management for dependencies
- **Makefile**: Convenience wrapper for common tasks

## Common Development Commands

### Building and Testing
```bash
# Build and run tests (recommended)
make build && make test

# Alternative manual build process
mkdir build_debug
cmake -S . -B build_debug -D CMAKE_BUILD_TYPE=Debug -D ENABLE_TESTING=ON -D CODE_COVERAGE=ON
cmake --build build_debug -t unit_tests_arffFiles -j 16
cd build_debug/tests && ./unit_tests_arffFiles
```

### Testing Options
```bash
# Run tests with verbose output
make test opt="-s"

# Clean test artifacts
make clean
```

### Code Coverage
Code coverage is enabled when building with `-D CODE_COVERAGE=ON` and `-D ENABLE_TESTING=ON`. Coverage reports are generated during test runs.

## Architecture

### Core Components

**Single Header Library**: `ArffFiles.hpp` contains the complete implementation.

**Main Class**: `ArffFiles`
- Header-only design for easy integration
- Handles ARFF file parsing and data conversion
- Automatically determines numeric vs categorical features
- Supports flexible class attribute positioning

### Key Methods
- `load(fileName, classLast=true)`: Load with class attribute at end/beginning
- `load(fileName, className)`: Load with specific named class attribute
- `getX()`: Returns feature vectors as `std::vector<std::vector<float>>`
- `getY()`: Returns labels as `std::vector<int>`
- `getNumericAttributes()`: Returns feature type mapping

### Data Processing Pipeline
1. **File Parsing**: Reads ARFF format, extracts attributes and data
2. **Feature Detection**: Automatically identifies numeric vs categorical attributes
3. **Preprocessing**: Handles missing values (lines with '?' are skipped)
4. **Factorization**: Converts categorical features to numeric codes
5. **Dataset Generation**: Creates final X (features) and y (labels) vectors

### Dependencies
- **Catch2**: Testing framework (fetched via CMake FetchContent)
- **Standard Library**: Uses STL containers (vector, map, string)
- **C++17**: Minimum required standard

### Test Structure
- Tests located in `tests/` directory
- Sample ARFF files in `tests/data/`
- Single test executable: `unit_tests_arffFiles`
- Uses Catch2 v3.3.2 for test framework

### Conan Integration
The project includes a `conanfile.py` that:
- Automatically extracts version from CMakeLists.txt
- Packages as a header-only library
- Exports only the main header file