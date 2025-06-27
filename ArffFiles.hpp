#ifndef ARFFFILES_HPP
#define ARFFFILES_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <cctype> // std::isdigit
#include <algorithm> // std::all_of std::transform

class ArffFiles {
    const std::string VERSION = "1.1.0";
public:
    ArffFiles() = default;
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
    std::vector<std::string> getLines() const { return lines; }
    unsigned long int getSize() const { return lines.size(); }
    std::string getClassName() const { return className; }
    std::string getClassType() const { return classType; }
    std::map<std::string, std::vector<std::string>> getStates() const { return states; }
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
    std::map<std::string, bool> getNumericAttributes() const { return numeric_features; }
    std::vector<std::pair<std::string, std::string>> getAttributes() const { return attributes; };
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
                        } catch (const std::exception& e) {
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
        file.close();
        
        // Final validation
        if (attributes.empty()) {
            throw std::invalid_argument("No attributes found in file");
        }
        if (lines.empty()) {
            throw std::invalid_argument("No data samples found in file");
        }
        
        // Initialize states for all attributes
        for (const auto& attribute : attributes) {
            states[attribute.first] = std::vector<std::string>();
        }
    }
    
    // Helper function for better missing value detection
    bool containsMissingValue(const std::string& line) {
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
};

#endif
