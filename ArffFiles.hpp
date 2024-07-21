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
        int labelIndex;
        loadCommon(fileName);
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
        preprocessDataset(labelIndex);
        generateDataset(labelIndex);
    }
    void load(const std::string& fileName, const std::string& name)
    {
        int labelIndex;
        loadCommon(fileName);
        bool found = false;
        for (int i = 0; i < attributes.size(); ++i) {
            if (attributes[i].first == name) {
                className = std::get<0>(attributes[i]);
                classType = std::get<1>(attributes[i]);
                attributes.erase(attributes.begin() + i);
                labelIndex = i;
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::invalid_argument("Class name not found");
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
    std::vector<int>& getY() { return y; }
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
    std::vector<std::vector<float>> X;
    std::vector<std::vector<std::string>> Xs;
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
        X = std::vector<std::vector<float>>(attributes.size(), std::vector<float>(lines.size()));
        Xs = std::vector<std::vector<std::string>>(attributes.size(), std::vector<std::string>(lines.size()));
        auto yy = std::vector<std::string>(lines.size(), "");
        for (size_t i = 0; i < lines.size(); i++) {
            std::stringstream ss(lines[i]);
            std::string value;
            int pos = 0;
            int xIndex = 0;
            auto tokens = split(lines[i], ',');
            for (const auto& token : tokens) {
                if (pos++ == labelIndex) {
                    yy[i] = token;
                } else {
                    if (numeric_features[attributes[xIndex].first]) {
                        X[xIndex][i] = stof(token);
                    } else {
                        Xs[xIndex][i] = token;
                    }
                    xIndex++;
                }
            }
        }
        for (size_t i = 0; i < attributes.size(); i++) {
            if (!numeric_features[attributes[i].first]) {
                auto data = factorize(attributes[i].first, Xs[i]);
                std::transform(data.begin(), data.end(), X[i].begin(), [](int x) { return float(x);});
            }
        }
        y = factorize(className, yy);
    }
    void loadCommon(std::string fileName)
    {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            throw std::invalid_argument("Unable to open file");
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
                type = "";
                while (ss >> type_w)
                    type += type_w + " ";
                attributes.emplace_back(trim(attribute), trim(type));
                continue;
            }
            if (line[0] == '@') {
                continue;
            }
            if (line.find("?", 0) != std::string::npos) {
                // ignore lines with missing values
                continue;
            }
            lines.push_back(line);
        }
        file.close();
        for (const auto& attribute : attributes) {
            states[attribute.first] = std::vector<std::string>();
        }
        if (attributes.empty())
            throw std::invalid_argument("No attributes found");
    }
};

#endif
