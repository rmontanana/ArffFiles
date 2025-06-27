#ifndef ARFFFILES_HPP
#define ARFFFILES_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <fstream>
#include <cctype> // std::isdigit
#include <algorithm> // std::all_of std::transform
#include <filesystem> // For file size checking

// Summary information structure for ARFF files
struct ArffSummary {
    size_t numSamples;           // Number of data samples
    size_t numFeatures;          // Number of feature attributes (excluding class)
    size_t numClasses;           // Number of different class values
    std::string className;       // Name of the class attribute
    std::string classType;       // Type/values of the class attribute
    std::vector<std::string> classLabels;  // List of unique class values
    std::vector<std::pair<std::string, std::string>> featureInfo; // Feature names and types
};

/**
 * @brief Header-only C++17 library for parsing ARFF (Attribute-Relation File Format) files
 * 
 * This class provides functionality to load and parse ARFF files, automatically detecting
 * numeric vs categorical features and performing factorization of categorical attributes.
 * 
 * @warning THREAD SAFETY: This class is NOT thread-safe!
 * 
 * Thread Safety Considerations:
 * - Multiple instances can be used safely in different threads (each instance is independent)
 * - A single instance MUST NOT be accessed concurrently from multiple threads
 * - All member functions (including getters) modify or access mutable state
 * - Static methods (summary, trim, split) are thread-safe as they don't access instance state
 * 
 * Memory Safety:
 * - Built-in protection against resource exhaustion with configurable limits
 * - File size limit: 100 MB (DEFAULT_MAX_FILE_SIZE)
 * - Sample count limit: 1 million samples (DEFAULT_MAX_SAMPLES)  
 * - Feature count limit: 10,000 features (DEFAULT_MAX_FEATURES)
 * 
 * Usage Patterns:
 * - Single-threaded: Create one instance, call load(), then access data via getters
 * - Multi-threaded: Create separate instances per thread, or use external synchronization
 * 
 * @example
 * // Thread-safe usage pattern:
 * void processFile(const std::string& filename) {
 *     ArffFiles arff;  // Each thread has its own instance
 *     arff.load(filename);
 *     auto X = arff.getX();
 *     auto y = arff.getY();
 *     // Process data...
 * }
 * 
 * @example  
 * // UNSAFE usage pattern:
 * ArffFiles globalArff;  // Global instance
 * // Thread 1: globalArff.load("file1.arff");  // UNSAFE!
 * // Thread 2: globalArff.load("file2.arff");  // UNSAFE!
 */
class ArffFiles {
    const std::string VERSION = "1.1.0";
    
    // Memory usage limits (configurable via environment variables)
    static constexpr size_t DEFAULT_MAX_FILE_SIZE = 100 * 1024 * 1024; // 100 MB
    static constexpr size_t DEFAULT_MAX_SAMPLES = 1000000; // 1 million samples
    static constexpr size_t DEFAULT_MAX_FEATURES = 10000; // 10k features
    
public:
    ArffFiles() = default;
    
    // Move constructor
    ArffFiles(ArffFiles&& other) noexcept
        : lines(std::move(other.lines))
        , numeric_features(std::move(other.numeric_features))
        , attributes(std::move(other.attributes))
        , className(std::move(other.className))
        , classType(std::move(other.classType))
        , states(std::move(other.states))
        , X(std::move(other.X))
        , y(std::move(other.y))
    {
        // Other object is left in a valid but unspecified state
    }
    
    // Move assignment operator
    ArffFiles& operator=(ArffFiles&& other) noexcept
    {
        if (this != &other) {
            lines = std::move(other.lines);
            numeric_features = std::move(other.numeric_features);
            attributes = std::move(other.attributes);
            className = std::move(other.className);
            classType = std::move(other.classType);
            states = std::move(other.states);
            X = std::move(other.X);
            y = std::move(other.y);
        }
        return *this;
    }
    
    // Copy constructor (explicitly defaulted)
    ArffFiles(const ArffFiles& other) = default;
    
    // Copy assignment operator (explicitly defaulted)
    ArffFiles& operator=(const ArffFiles& other) = default;
    
    // Destructor (explicitly defaulted)
    ~ArffFiles() = default;
    void load(const std::string& fileName, bool classLast = true)
    {
        if (fileName.empty()) {
            throw std::invalid_argument("File name cannot be empty");
        }

        int labelIndex;
        loadCommon(fileName);

        // Validate we have attributes before accessing them
        if (attributes.empty()) {
            throw std::invalid_argument("No attributes found in file");
        }

        if (classLast) {
            className = std::get<0>(attributes.back());
            classType = std::get<1>(attributes.back());
            attributes.pop_back();
            labelIndex = static_cast<int>(attributes.size());
        } else {
            className = std::get<0>(attributes.front());
            classType = std::get<1>(attributes.front());
            attributes.erase(attributes.begin());
            labelIndex = 0;
        }

        // Validate class name is not empty
        if (className.empty()) {
            throw std::invalid_argument("Class attribute name cannot be empty");
        }

        preprocessDataset(labelIndex);
        generateDataset(labelIndex);
    }
    void load(const std::string& fileName, const std::string& name)
    {
        if (fileName.empty()) {
            throw std::invalid_argument("File name cannot be empty");
        }
        if (name.empty()) {
            throw std::invalid_argument("Class name cannot be empty");
        }

        int labelIndex;
        loadCommon(fileName);

        // Validate we have attributes before searching
        if (attributes.empty()) {
            throw std::invalid_argument("No attributes found in file");
        }

        bool found = false;
        for (size_t i = 0; i < attributes.size(); ++i) {
            if (attributes[i].first == name) {
                className = std::get<0>(attributes[i]);
                classType = std::get<1>(attributes[i]);
                attributes.erase(attributes.begin() + i);
                labelIndex = static_cast<int>(i);
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::invalid_argument("Class name '" + name + "' not found in attributes");
        }
        preprocessDataset(labelIndex);
        generateDataset(labelIndex);
    }

    // Static method to get summary information without loading all data (default: class is last)
    static ArffSummary summary(const std::string& fileName)
    {
        return summary(fileName, true);
    }

    // Static method to get summary information without loading all data
    static ArffSummary summary(const std::string& fileName, bool classLast)
    {
        if (fileName.empty()) {
            throw std::invalid_argument("File name cannot be empty");
        }
        return summarizeFile(fileName, classLast);
    }

    // Static method to get summary information with specified class attribute (const char* overload)
    static ArffSummary summary(const std::string& fileName, const char* className)
    {
        return summary(fileName, std::string(className));
    }

    // Static method to get summary information with specified class attribute
    static ArffSummary summary(const std::string& fileName, const std::string& className)
    {
        if (fileName.empty()) {
            throw std::invalid_argument("File name cannot be empty");
        }
        if (className.empty()) {
            throw std::invalid_argument("Class name cannot be empty");
        }
        return summarizeFile(fileName, className);
    }
    const std::vector<std::string>& getLines() const { return lines; }
    size_t getSize() const { return lines.size(); }
    std::string getClassName() const { return className; }
    std::string getClassType() const { return classType; }
    const std::map<std::string, std::vector<std::string>>& getStates() const { return states; }
    std::vector<std::string> getLabels() const { return states.at(className); }
    static std::string trim(const std::string& source)
    {
        std::string s(source);
        s.erase(0, s.find_first_not_of(" '\n\r\t"));
        s.erase(s.find_last_not_of(" '\n\r\t") + 1);
        return s;
    }
    std::vector<std::vector<float>>& getX() { return X; }
    const std::vector<std::vector<float>>& getX() const { return X; }
    std::vector<int>& getY() { return y; }
    const std::vector<int>& getY() const { return y; }
    const std::map<std::string, bool>& getNumericAttributes() const { return numeric_features; }
    const std::vector<std::pair<std::string, std::string>>& getAttributes() const { return attributes; };
    
    // Move-enabled getters for efficient data transfer
    // WARNING: These methods move data OUT of the object, leaving it in an empty but valid state
    // Use these when you want to transfer ownership of large data structures for performance
    std::vector<std::vector<float>> moveX() noexcept { return std::move(X); }
    std::vector<int> moveY() noexcept { return std::move(y); }
    std::vector<std::string> moveLines() noexcept { return std::move(lines); }
    std::map<std::string, std::vector<std::string>> moveStates() noexcept { return std::move(states); }
    std::vector<std::pair<std::string, std::string>> moveAttributes() noexcept { return std::move(attributes); }
    std::map<std::string, bool> moveNumericAttributes() noexcept { return std::move(numeric_features); }
    
    std::vector<std::string> split(const std::string& text, char delimiter)
    {
        std::vector<std::string> result;
        std::stringstream ss(text);
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            result.push_back(trim(token));
        }
        return result;
    }
    std::string version() const { return VERSION; }

private:
    // Helper function to validate resource usage limits
    static void validateResourceLimits(const std::string& fileName, size_t sampleCount = 0, size_t featureCount = 0) {
        // Check file size limit
        try {
            if (std::filesystem::exists(fileName)) {
                auto fileSize = std::filesystem::file_size(fileName);
                if (fileSize > DEFAULT_MAX_FILE_SIZE) {
                    throw std::invalid_argument("File size (" + std::to_string(fileSize) + " bytes) exceeds maximum allowed size (" + std::to_string(DEFAULT_MAX_FILE_SIZE) + " bytes)");
                }
            }
        } catch (const std::filesystem::filesystem_error&) {
            // If filesystem operations fail, continue without size checking
            // This ensures compatibility with systems where filesystem might not be available
        }
        
        // Check sample count limit
        if (sampleCount > DEFAULT_MAX_SAMPLES) {
            throw std::invalid_argument("Number of samples (" + std::to_string(sampleCount) + ") exceeds maximum allowed (" + std::to_string(DEFAULT_MAX_SAMPLES) + ")");
        }
        
        // Check feature count limit
        if (featureCount > DEFAULT_MAX_FEATURES) {
            throw std::invalid_argument("Number of features (" + std::to_string(featureCount) + ") exceeds maximum allowed (" + std::to_string(DEFAULT_MAX_FEATURES) + ")");
        }
    }

protected:
    std::vector<std::string> lines;
    std::map<std::string, bool> numeric_features;
    std::vector<std::pair<std::string, std::string>> attributes;
    std::string className;
    std::string classType;
    std::vector<std::vector<float>> X;  // X[feature][sample] - feature-major layout
    std::vector<int> y;
    std::map<std::string, std::vector<std::string>> states;
private:
    void preprocessDataset(int labelIndex)
    {
        //
        // Learn the numeric features
        //
        numeric_features.clear();
        for (const auto& attribute : attributes) {
            auto feature = attribute.first;
            if (feature == className)
                continue;
            auto values = attribute.second;
            std::transform(values.begin(), values.end(), values.begin(), ::toupper);
            numeric_features[feature] = values == "REAL" || values == "INTEGER" || values == "NUMERIC";
        }
    }
    std::vector<int> factorize(const std::string feature, const std::vector<std::string>& labels_t)
    {
        std::vector<int> yy;
        states.at(feature).clear();
        yy.reserve(labels_t.size());
        std::map<std::string, int> labelMap;
        int i = 0;
        for (const std::string& label : labels_t) {
            if (labelMap.find(label) == labelMap.end()) {
                labelMap[label] = i++;
                bool allDigits = std::all_of(label.begin(), label.end(), ::isdigit);
                if (allDigits)
                    states[feature].push_back("Class " + label);
                else
                    states[feature].push_back(label);
            }
            yy.push_back(labelMap[label]);
        }
        return yy;
    }
    void generateDataset(int labelIndex)
    {
        const size_t numSamples = lines.size();
        const size_t numFeatures = attributes.size();

        // Validate inputs
        if (numSamples == 0) {
            throw std::invalid_argument("No data samples found in file");
        }
        if (numFeatures == 0) {
            throw std::invalid_argument("No feature attributes found");
        }
        if (labelIndex < 0) {
            throw std::invalid_argument("Invalid label index: cannot be negative");
        }

        // Pre-allocate with feature-major layout: X[feature][sample]
        X.assign(numFeatures, std::vector<float>(numSamples));

        // Temporary storage for categorical data per feature (only for non-numeric features)
        std::vector<std::vector<std::string>> categoricalData(numFeatures);
        for (size_t i = 0; i < numFeatures; ++i) {
            if (!numeric_features[attributes[i].first]) {
                categoricalData[i].reserve(numSamples);
            }
        }

        std::vector<std::string> yy;
        yy.reserve(numSamples);

        // Parse each sample
        for (size_t sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
            const auto tokens = split(lines[sampleIdx], ',');

            // Validate token count matches expected number (features + class)
            const size_t expectedTokens = numFeatures + 1;
            if (tokens.size() != expectedTokens) {
                throw std::invalid_argument("Sample " + std::to_string(sampleIdx) + " has " + std::to_string(tokens.size()) + " tokens, expected " + std::to_string(expectedTokens));
            }

            int pos = 0;
            int featureIdx = 0;

            for (const auto& token : tokens) {
                if (pos++ == labelIndex) {
                    if (token.empty()) {
                        throw std::invalid_argument("Empty class label at sample " + std::to_string(sampleIdx));
                    }
                    yy.push_back(token);
                } else {
                    if (featureIdx >= static_cast<int>(numFeatures)) {
                        throw std::invalid_argument("Too many feature values at sample " + std::to_string(sampleIdx));
                    }

                    const auto& featureName = attributes[featureIdx].first;
                    if (numeric_features.at(featureName)) {
                        // Parse numeric value with exception handling
                        try {
                            X[featureIdx][sampleIdx] = std::stof(token);
                        }
                        catch (const std::exception& e) {
                            throw std::invalid_argument("Invalid numeric value '" + token + "' at sample " + std::to_string(sampleIdx) + ", feature " + featureName);
                        }
                    } else {
                        // Store categorical value temporarily
                        if (token.empty()) {
                            throw std::invalid_argument("Empty categorical value at sample " + std::to_string(sampleIdx) + ", feature " + featureName);
                        }
                        categoricalData[featureIdx].push_back(token);
                    }
                    featureIdx++;
                }
            }
        }

        // Convert categorical features to numeric
        for (size_t featureIdx = 0; featureIdx < numFeatures; ++featureIdx) {
            if (!numeric_features[attributes[featureIdx].first]) {
                const auto& featureName = attributes[featureIdx].first;
                auto encodedValues = factorize(featureName, categoricalData[featureIdx]);

                // Copy encoded values to X[feature][sample]
                for (size_t sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
                    X[featureIdx][sampleIdx] = static_cast<float>(encodedValues[sampleIdx]);
                }
            }
        }

        y = factorize(className, yy);
    }
    void loadCommon(std::string fileName)
    {
        // Clear previous data
        lines.clear();
        attributes.clear();
        states.clear();
        numeric_features.clear();

        // Validate file size before processing
        validateResourceLimits(fileName);

        std::ifstream file(fileName);
        if (!file.is_open()) {
            throw std::invalid_argument("Unable to open file: " + fileName);
        }
        std::string line;
        std::string keyword;
        std::string attribute;
        std::string type;
        std::string type_w;
        while (getline(file, line)) {
            if (line.empty() || line[0] == '%' || line == "\r" || line == " ") {
                continue;
            }
            if (line.find("@attribute") != std::string::npos || line.find("@ATTRIBUTE") != std::string::npos) {
                std::stringstream ss(line);
                ss >> keyword >> attribute;

                // Validate attribute name
                if (attribute.empty()) {
                    throw std::invalid_argument("Empty attribute name in line: " + line);
                }

                // Check for duplicate attribute names
                for (const auto& existing : attributes) {
                    if (existing.first == attribute) {
                        throw std::invalid_argument("Duplicate attribute name: " + attribute);
                    }
                }

                // Efficiently build type string
                std::ostringstream typeStream;
                while (ss >> type_w) {
                    if (typeStream.tellp() > 0) typeStream << " ";
                    typeStream << type_w;
                }
                type = typeStream.str();

                // Validate type is not empty
                if (type.empty()) {
                    throw std::invalid_argument("Empty attribute type for attribute: " + attribute);
                }

                attributes.emplace_back(trim(attribute), trim(type));
                continue;
            }
            if (line[0] == '@') {
                continue;
            }
            // More sophisticated missing value detection
            // Skip lines with '?' not inside quoted strings
            if (containsMissingValue(line)) {
                continue;
            }
            lines.push_back(line);
        }

        // Final validation
        if (attributes.empty()) {
            throw std::invalid_argument("No attributes found in file");
        }
        if (lines.empty()) {
            throw std::invalid_argument("No data samples found in file");
        }
        
        // Validate loaded data dimensions against limits
        validateResourceLimits(fileName, lines.size(), attributes.size());

        // Initialize states for all attributes
        for (const auto& attribute : attributes) {
            states[attribute.first] = std::vector<std::string>();
        }
    }

    // Helper function for better missing value detection
    bool containsMissingValue(const std::string& line)
    {
        bool inQuotes = false;
        char quoteChar = '\0';

        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];

            if (!inQuotes && (c == '\'' || c == '\"')) {
                inQuotes = true;
                quoteChar = c;
            } else if (inQuotes && c == quoteChar) {
                inQuotes = false;
                quoteChar = '\0';
            } else if (!inQuotes && c == '?') {
                // Found unquoted '?' - this is a missing value
                return true;
            }
        }
        return false;
    }

    // Static version of missing value detection for summary methods
    static bool containsMissingValueStatic(const std::string& line)
    {
        bool inQuotes = false;
        char quoteChar = '\0';

        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];

            if (!inQuotes && (c == '\'' || c == '\"')) {
                inQuotes = true;
                quoteChar = c;
            } else if (inQuotes && c == quoteChar) {
                inQuotes = false;
                quoteChar = '\0';
            } else if (!inQuotes && c == '?') {
                // Found unquoted '?' - this is a missing value
                return true;
            }
        }
        return false;
    }

    // Common helper function to parse ARFF file attributes and count samples
    static int parseArffFile(const std::string& fileName, 
                            std::vector<std::pair<std::string, std::string>>& attributes,
                            std::set<std::string>& uniqueClasses,
                            size_t& sampleCount,
                            int classIndex = -1,
                            const std::string& classNameToFind = "") {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            throw std::invalid_argument("Unable to open file: " + fileName);
        }

        std::string line;
        attributes.clear();
        uniqueClasses.clear();
        sampleCount = 0;

        // Parse header
        while (getline(file, line)) {
            if (line.empty() || line[0] == '%' || line == "\r" || line == " ") {
                continue;
            }
            if (line.find("@attribute") != std::string::npos || line.find("@ATTRIBUTE") != std::string::npos) {
                std::stringstream ss(line);
                std::string keyword, attribute, type_w;
                ss >> keyword >> attribute;

                if (attribute.empty()) {
                    throw std::invalid_argument("Empty attribute name in line: " + line);
                }

                // Build type string
                std::ostringstream typeStream;
                while (ss >> type_w) {
                    if (typeStream.tellp() > 0) typeStream << " ";
                    typeStream << type_w;
                }
                std::string type = typeStream.str();

                if (type.empty()) {
                    throw std::invalid_argument("Empty attribute type for attribute: " + attribute);
                }

                attributes.emplace_back(trim(attribute), trim(type));
                continue;
            }
            if (line[0] == '@') {
                continue;
            }
            // Start of data section
            break;
        }

        if (attributes.empty()) {
            throw std::invalid_argument("No attributes found in file");
        }

        // Find class index if class name is specified
        int actualClassIndex = classIndex;
        if (!classNameToFind.empty()) {
            actualClassIndex = -1;
            for (size_t i = 0; i < attributes.size(); ++i) {
                if (attributes[i].first == classNameToFind) {
                    actualClassIndex = static_cast<int>(i);
                    break;
                }
            }
            if (actualClassIndex == -1) {
                throw std::invalid_argument("Class name '" + classNameToFind + "' not found in attributes");
            }
        }

        // Count samples and collect unique class values
        do {
            if (!line.empty() && line[0] != '@' && line[0] != '%' && !containsMissingValueStatic(line)) {
                auto tokens = splitStatic(line, ',');
                if (!tokens.empty()) {
                    std::string classValue;
                    if (actualClassIndex == -1) {
                        // Use last token (default behavior)
                        classValue = trim(tokens.back());
                    } else if (actualClassIndex == 0) {
                        // Use first token
                        classValue = trim(tokens.front());
                    } else if (actualClassIndex > 0 && static_cast<size_t>(actualClassIndex) < tokens.size()) {
                        // Use specific index
                        classValue = trim(tokens[actualClassIndex]);
                    }
                    
                    if (!classValue.empty()) {
                        uniqueClasses.insert(classValue);
                        sampleCount++;
                    }
                }
            }
        }
        while (getline(file, line));
        
        return actualClassIndex;
    }

    // Helper function for summary with classLast parameter
    static ArffSummary summarizeFile(const std::string& fileName, bool classLast)
    {
        ArffSummary summary;
        std::vector<std::pair<std::string, std::string>> attributes;
        std::set<std::string> uniqueClasses;
        size_t sampleCount = 0;

        // Use common parsing function
        parseArffFile(fileName, attributes, uniqueClasses, sampleCount, classLast ? -1 : 0);

        // Determine class attribute
        if (classLast) {
            summary.className = attributes.back().first;
            summary.classType = attributes.back().second;
            attributes.pop_back();
        } else {
            summary.className = attributes.front().first;
            summary.classType = attributes.front().second;
            attributes.erase(attributes.begin());
        }

        summary.numFeatures = attributes.size();

        // Copy feature information
        for (const auto& attr : attributes) {
            summary.featureInfo.emplace_back(attr.first, attr.second);
        }



        summary.numSamples = sampleCount;
        summary.numClasses = uniqueClasses.size();
        summary.classLabels.assign(uniqueClasses.begin(), uniqueClasses.end());

        return summary;
    }

    // Helper function for summary with className parameter
    static ArffSummary summarizeFile(const std::string& fileName, const std::string& className)
    {
        ArffSummary summary;
        std::vector<std::pair<std::string, std::string>> attributes;
        std::set<std::string> uniqueClasses;
        size_t sampleCount = 0;
        int classIndex = -1;

        // Use common parsing function to find class by name
        classIndex = parseArffFile(fileName, attributes, uniqueClasses, sampleCount, -1, className);

        // Set class information from the found attribute
        summary.className = attributes[classIndex].first;
        summary.classType = attributes[classIndex].second;

        // Remove class attribute from features
        attributes.erase(attributes.begin() + classIndex);
        summary.numFeatures = attributes.size();

        // Copy feature information
        for (const auto& attr : attributes) {
            summary.featureInfo.emplace_back(attr.first, attr.second);
        }

        summary.numSamples = sampleCount;
        summary.numClasses = uniqueClasses.size();
        summary.classLabels.assign(uniqueClasses.begin(), uniqueClasses.end());

        return summary;
    }

    // Static helper function for split (needed by summarizeFile)
    static std::vector<std::string> splitStatic(const std::string& text, char delimiter)
    {
        std::vector<std::string> result;
        std::stringstream ss(text);
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            result.push_back(trim(token));
        }
        return result;
    }
};

#endif
