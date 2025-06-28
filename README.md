# <img src="logo.png" alt="logo" width="50"/> ArffFiles

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=flat&logo=c%2B%2B&logoColor=white)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](<https://opensource.org/licenses/MIT>)
![Gitea Release](https://img.shields.io/gitea/v/release/rmontanana/arfffiles?gitea_url=https://gitea.rmontanana.es)
![Gitea Last Commit](https://img.shields.io/gitea/last-commit/rmontanana/arfffiles?gitea_url=https://gitea.rmontanana.es&logo=gitea)

A modern C++17 header-only library to read **ARFF (Attribute-Relation File Format)** files and convert them into STL vectors for machine learning and data analysis applications.

## Features

- 🔧 **Header-only**: Simply include `ArffFiles.hpp` - no compilation required
- 🚀 **Modern C++17**: Clean, efficient implementation using modern C++ standards
- 🔄 **Automatic Type Detection**: Distinguishes between numeric and categorical attributes
- 📊 **Flexible Class Positioning**: Support for class attributes at any position
- 🎯 **STL Integration**: Returns standard `std::vector` containers for seamless integration
- 🧹 **Data Cleaning**: Automatically handles missing values (lines with '?' are skipped)
- 🏷️ **Label Encoding**: Automatic factorization of categorical features into numeric codes

## Requirements

- **C++17** compatible compiler
- **Standard Library**: Uses STL containers (no external dependencies)

## Installation

### Using Conan

```bash
# Add the package to your conanfile.txt
[requires]
arff-files/1.2.0

# Or install directly
conan install arff-files/1.2.0@
```

### Manual Installation

Simply download `ArffFiles.hpp` and include it in your project:

```cpp
#include "ArffFiles.hpp"
```

## Quick Start

```cpp
#include "ArffFiles.hpp"
#include <iostream>

int main() {
    ArffFiles arff;
    
    // Load ARFF file (class attribute at the end by default)
    arff.load("dataset.arff");
    
    // Get feature matrix and labels
    auto& X = arff.getX();  // std::vector<std::vector<float>>
    auto& y = arff.getY();  // std::vector<int>
    
    std::cout << "Dataset size: " << arff.getSize() << " samples" << std::endl;
    std::cout << "Features: " << X.size() << std::endl;
    std::cout << "Classes: " << arff.getLabels().size() << std::endl;
    
    return 0;
}
```

## API Reference

### Loading Data

```cpp
// Load with class attribute at the end (default)
arff.load("dataset.arff");

// Load with class attribute at the beginning
arff.load("dataset.arff", false);

// Load with specific named class attribute
arff.load("dataset.arff", "class_name");
```

### Accessing Data

```cpp
// Get feature matrix (each inner vector is a feature, not a sample)
std::vector<std::vector<float>>& X = arff.getX();

// Get labels (encoded as integers)
std::vector<int>& y = arff.getY();

// Get dataset information
std::string className = arff.getClassName();
std::vector<std::string> labels = arff.getLabels();
unsigned long size = arff.getSize();

// Get attribute information
auto attributes = arff.getAttributes();  // std::vector<std::pair<std::string, std::string>>
auto numericFeatures = arff.getNumericAttributes();  // std::map<std::string, bool>
```

### Utility Methods

```cpp
// Get library version
std::string version = arff.version();

// Access raw lines (after preprocessing)
std::vector<std::string> lines = arff.getLines();

// Get label states mapping
auto states = arff.getStates();  // std::map<std::string, std::vector<std::string>>
```

## Data Processing Pipeline

1. **File Parsing**: Reads ARFF format, extracts `@attribute` declarations and data
2. **Missing Value Handling**: Skips lines containing `?` (missing values)
3. **Feature Type Detection**: Automatically identifies `REAL`, `INTEGER`, `NUMERIC` vs categorical
4. **Label Positioning**: Handles class attributes at any position in the data
5. **Factorization**: Converts categorical features and labels to numeric codes
6. **Data Organization**: Creates feature matrix `X` and label vector `y`

## Example: Complete Workflow

```cpp
#include "ArffFiles.hpp"
#include <iostream>

int main() {
    try {
        ArffFiles arff;
        arff.load("iris.arff");
        
        // Display dataset information
        std::cout << "Dataset: " << arff.getClassName() << std::endl;
        std::cout << "Samples: " << arff.getSize() << std::endl;
        std::cout << "Features: " << arff.getX().size() << std::endl;
        
        // Show class labels
        auto labels = arff.getLabels();
        std::cout << "Classes: ";
        for (const auto& label : labels) {
            std::cout << label << " ";
        }
        std::cout << std::endl;
        
        // Show which features are numeric
        auto numericFeatures = arff.getNumericAttributes();
        for (const auto& [feature, isNumeric] : numericFeatures) {
            std::cout << feature << ": " << (isNumeric ? "numeric" : "categorical") << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Supported ARFF Features

- ✅ Numeric attributes (`@attribute feature REAL/INTEGER/NUMERIC`)
- ✅ Categorical attributes (`@attribute feature {value1,value2,...}`)
- ✅ Comments (lines starting with `%`)
- ✅ Missing values (automatic skipping of lines with `?`)
- ✅ Flexible class attribute positioning
- ✅ Case-insensitive attribute declarations

## Error Handling

The library throws `std::invalid_argument` exceptions for:
- Unable to open file
- No attributes found in file
- Specified class name not found

## Development

### Building and Testing

```bash
# Build and run tests
make build && make test

# Run tests with verbose output
make test opt="-s"

# Clean test artifacts
make clean
```

### Using CMake Directly

```bash
mkdir build_debug
cmake -S . -B build_debug -D CMAKE_BUILD_TYPE=Debug -D ENABLE_TESTING=ON
cmake --build build_debug -t unit_tests_arffFiles
cd build_debug/tests && ./unit_tests_arffFiles
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
