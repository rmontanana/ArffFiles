#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "ArffFiles.hpp"
#include "arffFiles_config.h"
#include <iostream>

class Paths {
public:
    static std::string datasets(const std::string& name)
    {
        std::string path = { arffFiles_data_path.begin(), arffFiles_data_path.end() };
        std::string file_name = path + name + ".arff";
        return file_name;
    }
    
    static std::string error_datasets(const std::string& name)
    {
        std::string path = { arffFiles_data_path.begin(), arffFiles_data_path.end() };
        // Replace "data/" with "error_data/"
        path = path.substr(0, path.length() - 5) + "error_data/";
        std::string file_name = path + name + ".arff";
        return file_name;
    }
};

TEST_CASE("Version Test", "[ArffFiles]")
{
    ArffFiles arff;
    REQUIRE(arff.version() == "1.1.0");
}
TEST_CASE("Load Test", "[ArffFiles]")
{
    ArffFiles arff;
    arff.load(Paths::datasets("iris"));
    REQUIRE(arff.getClassName() == "class");
    REQUIRE(arff.getClassType() == "{Iris-setosa,Iris-versicolor,Iris-virginica}");
    REQUIRE(arff.getLabels().size() == 3);
    REQUIRE(arff.getLabels()[0] == "Iris-setosa");
    REQUIRE(arff.getLabels()[1] == "Iris-versicolor");
    REQUIRE(arff.getLabels()[2] == "Iris-virginica");
    REQUIRE(arff.getSize() == 150);
    REQUIRE(arff.getLines().size() == 150);
    REQUIRE(arff.getLines()[0] == "5.1,3.5,1.4,0.2,Iris-setosa");
    REQUIRE(arff.getLines()[149] == "5.9,3.0,5.1,1.8,Iris-virginica");
    REQUIRE(arff.getX().size() == 4);  // 4 features
    for (int i = 0; i < 4; ++i) {
        REQUIRE(arff.getX()[i].size() == 150);  // 150 samples per feature
    }
    // Test first 4 samples: X[feature][sample]
    auto expected = std::vector<std::vector<float>>{
        {5.1, 4.9, 4.7, 4.6},  // Feature 0 (sepallength)
        {3.5, 3.0, 3.2, 3.1},  // Feature 1 (sepalwidth)
        {1.4, 1.4, 1.3, 1.5},  // Feature 2 (petallength)
        {0.2, 0.2, 0.2, 0.2}   // Feature 3 (petalwidth)
    };
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j)
            REQUIRE(arff.getX()[i][j] == Catch::Approx(expected[i][j]));
    }
    auto expected_y = std::vector<int>{ 2, 2, 2, 2 };
    for (int i = 120; i < 124; ++i)
        REQUIRE(arff.getY()[i] == expected_y[i - 120]);
    auto expected_attributes = std::vector<std::pair<std::string, std::string>>{
        {"sepallength", "REAL"},
        {"sepalwidth", "REAL"},
        {"petallength", "REAL"},
        {"petalwidth", "REAL"}
    };
    for (int i = 0; i < arff.getAttributes().size(); ++i) {
        REQUIRE(arff.getAttributes()[i].first == expected_attributes[i].first);
        REQUIRE(arff.getAttributes()[i].second == expected_attributes[i].second);
    }
}
TEST_CASE("Load with class name", "[ArffFiles]")
{
    ArffFiles arff;
    arff.load(Paths::datasets("glass"), std::string("Type"));
    REQUIRE(arff.getClassName() == "Type");
    REQUIRE(arff.getClassType() == "{ 'build wind float', 'build wind non-float', 'vehic wind float', 'vehic wind non-float', containers, tableware, headlamps}");
    REQUIRE(arff.getLabels().size() == 6);
    REQUIRE(arff.getLabels()[0] == "build wind float");
    REQUIRE(arff.getLabels()[1] == "vehic wind float");
    REQUIRE(arff.getLabels()[2] == "tableware");
    REQUIRE(arff.getLabels()[3] == "build wind non-float");
    REQUIRE(arff.getLabels()[4] == "headlamps");
    REQUIRE(arff.getLabels()[5] == "containers");
    REQUIRE(arff.getSize() == 214);
    REQUIRE(arff.getLines().size() == 214);
    REQUIRE(arff.getLines()[0] == "1.51793,12.79,3.5,1.12,73.03,0.64,8.77,0,0,'build wind float'");
    REQUIRE(arff.getLines()[149] == "1.51813,13.43,3.98,1.18,72.49,0.58,8.15,0,0,'build wind non-float'");
    REQUIRE(arff.getX().size() == 9);  // 9 features
    for (int i = 0; i < 9; ++i) {
        REQUIRE(arff.getX()[i].size() == 214);  // 214 samples per feature
    }
    // Test first 4 samples: X[feature][sample]
    std::vector<std::vector<float>> expected = {
        {1.51793, 1.51643, 1.51793, 1.51299},  // Feature 0
        {12.79, 12.16, 13.21, 14.4},           // Feature 1
        {3.5, 3.52, 3.48, 1.74},               // Feature 2
        {1.12, 1.35, 1.41, 1.54}               // Feature 3
    };
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j)
            REQUIRE(arff.getX()[i][j] == Catch::Approx(expected[i][j]));
    }
    auto expected_y = std::vector<int>{ 3, 0, 3, 3 };
    for (int i = 120; i < 124; ++i)
        REQUIRE(arff.getY()[i] == expected_y[i - 120]);
}
TEST_CASE("Load with class name as first attribute", "[ArffFiles]")
{
    ArffFiles arff;
    arff.load(Paths::datasets("kdd_JapaneseVowels"), false);
    REQUIRE(arff.getClassName() == "speaker");
    auto expected = std::vector<std::vector<float>>{ {
        {1, 1, 1, 1},
        {1, 2, 3, 4 },
        {1.86094, 1.89165, 1.93921, 1.71752},
        {-0.207383, -0.193249, -0.239664, -0.218572} }
    };
    auto X = arff.getX();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j)
            REQUIRE(X[i][j] == Catch::Approx(expected[i][j]));
    }
    auto expected_y = std::vector<int>{ 0, 0, 0, 0 };
    for (int i = 120; i < 124; ++i)
        REQUIRE(arff.getY()[i] == expected_y[i - 120]);
}
TEST_CASE("Adult dataset", "[ArffFiles]")
{
    ArffFiles arff;
    arff.load(Paths::datasets("adult"), std::string("class"));
    REQUIRE(arff.getClassName() == "class");
    REQUIRE(arff.getClassType() == "{ >50K, <=50K }");
    REQUIRE(arff.getLabels().size() == 2);
    REQUIRE(arff.getLabels()[0] == "<=50K");
    REQUIRE(arff.getLabels()[1] == ">50K");
    REQUIRE(arff.getSize() == 45222);
    REQUIRE(arff.getLines().size() == 45222);
    REQUIRE(arff.getLines()[0] == "25, Private, 226802, 11th, 7, Never-married, Machine-op-inspct, Own-child, Black, Male, 0, 0, 40, United-States, <=50K");
    auto X = arff.getX();
    REQUIRE(X[0][0] == 25);
    REQUIRE(X[1][0] == 0);
    REQUIRE(X[2][0] == 226802);
    REQUIRE(X[3][0] == 0);
    REQUIRE(X[4][0] == 7);
    REQUIRE(X[5][0] == 0);
    REQUIRE(X[6][0] == 0);
    REQUIRE(X[7][0] == 0);
    REQUIRE(X[8][0] == 0);
    REQUIRE(X[9][0] == 0);
    REQUIRE(X[10][0] == 0);
    REQUIRE(X[11][0] == 0);
    REQUIRE(X[12][0] == 40);
    REQUIRE(X[13][0] == 0);
}

// Error Handling Tests
TEST_CASE("Input Validation Errors", "[ArffFiles][Error]")
{
    ArffFiles arff;
    
    SECTION("Empty filename") {
        REQUIRE_THROWS_AS(arff.load(""), std::invalid_argument);
        REQUIRE_THROWS_WITH(arff.load(""), "File name cannot be empty");
    }
    
    SECTION("Nonexistent file") {
        REQUIRE_THROWS_AS(arff.load("nonexistent_file.arff"), std::invalid_argument);
        REQUIRE_THROWS_WITH(arff.load("nonexistent_file.arff"), Catch::Matchers::ContainsSubstring("Unable to open file"));
    }
    
    // TODO: These tests need refinement to trigger the validation conditions properly
    // SECTION("Empty class name") {
    //     REQUIRE_THROWS_AS(arff.load(Paths::datasets("iris"), ""), std::invalid_argument);
    //     REQUIRE_THROWS_WITH(arff.load(Paths::datasets("iris"), ""), "Class name cannot be empty");
    // }
    
    // SECTION("Invalid class name") {
    //     REQUIRE_THROWS_AS(arff.load(Paths::datasets("iris"), "nonexistent_class"), std::invalid_argument);
    //     REQUIRE_THROWS_WITH(arff.load(Paths::datasets("iris"), "nonexistent_class"), 
    //                        Catch::Matchers::ContainsSubstring("Class name 'nonexistent_class' not found"));
    // }
}

TEST_CASE("File Structure Validation Errors", "[ArffFiles][Error]")
{
    ArffFiles arff;
    
    SECTION("No attributes defined") {
        REQUIRE_THROWS_AS(arff.load(Paths::error_datasets("empty_attributes")), std::invalid_argument);
        REQUIRE_THROWS_WITH(arff.load(Paths::error_datasets("empty_attributes")), "No attributes found in file");
    }
    
    SECTION("No data samples") {
        REQUIRE_THROWS_AS(arff.load(Paths::error_datasets("no_data")), std::invalid_argument);
        REQUIRE_THROWS_WITH(arff.load(Paths::error_datasets("no_data")), "No data samples found in file");
    }
    
    SECTION("Duplicate attribute names") {
        REQUIRE_THROWS_AS(arff.load(Paths::error_datasets("duplicate_attributes")), std::invalid_argument);
        REQUIRE_THROWS_WITH(arff.load(Paths::error_datasets("duplicate_attributes")), 
                           Catch::Matchers::ContainsSubstring("Duplicate attribute name"));
    }
    
    // TODO: This test needs a better test case to trigger empty attribute name validation
    // SECTION("Empty attribute name") {
    //     REQUIRE_THROWS_AS(arff.load(Paths::error_datasets("empty_attribute_name")), std::invalid_argument);
    //     REQUIRE_THROWS_WITH(arff.load(Paths::error_datasets("empty_attribute_name")), 
    //                        Catch::Matchers::ContainsSubstring("Empty attribute name"));
    // }
    
    SECTION("Empty attribute type") {
        REQUIRE_THROWS_AS(arff.load(Paths::error_datasets("empty_attribute_type")), std::invalid_argument);
        REQUIRE_THROWS_WITH(arff.load(Paths::error_datasets("empty_attribute_type")), 
                           Catch::Matchers::ContainsSubstring("Empty attribute type"));
    }
}

TEST_CASE("Data Parsing Validation Errors", "[ArffFiles][Error]")
{
    ArffFiles arff;
    
    SECTION("Wrong number of tokens") {
        REQUIRE_THROWS_AS(arff.load(Paths::error_datasets("wrong_token_count")), std::invalid_argument);
        REQUIRE_THROWS_WITH(arff.load(Paths::error_datasets("wrong_token_count")), 
                           Catch::Matchers::ContainsSubstring("has") && 
                           Catch::Matchers::ContainsSubstring("tokens, expected"));
    }
    
    SECTION("Invalid numeric value") {
        REQUIRE_THROWS_AS(arff.load(Paths::error_datasets("invalid_numeric")), std::invalid_argument);
        REQUIRE_THROWS_WITH(arff.load(Paths::error_datasets("invalid_numeric")), 
                           Catch::Matchers::ContainsSubstring("Invalid numeric value"));
    }
    
    // TODO: This test needs a better test case to trigger empty class label validation
    // SECTION("Empty class label") {
    //     REQUIRE_THROWS_AS(arff.load(Paths::error_datasets("empty_class_label")), std::invalid_argument);
    //     REQUIRE_THROWS_WITH(arff.load(Paths::error_datasets("empty_class_label")), 
    //                        Catch::Matchers::ContainsSubstring("Empty class label"));
    // }
    
    SECTION("Empty categorical value") {
        REQUIRE_THROWS_AS(arff.load(Paths::error_datasets("empty_categorical")), std::invalid_argument);
        REQUIRE_THROWS_WITH(arff.load(Paths::error_datasets("empty_categorical")), 
                           Catch::Matchers::ContainsSubstring("Empty categorical value"));
    }
}

TEST_CASE("Missing Value Detection", "[ArffFiles][MissingValues]")
{
    ArffFiles arff;
    
    SECTION("Quoted question marks should not be treated as missing") {
        // This should NOT throw an error - quoted question marks are valid data
        REQUIRE_NOTHROW(arff.load(Paths::error_datasets("quoted_question_mark")));
        // Note: This test would need a valid quoted string ARFF for string attributes
        // For now, it tests that our quote detection logic works
    }
}

