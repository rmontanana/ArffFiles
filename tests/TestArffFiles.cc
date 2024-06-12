#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
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
    REQUIRE(arff.getX().size() == 4);
    for (int i = 0; i < 4; ++i) {
        REQUIRE(arff.getX()[i].size() == 150);
    }
    auto expected = std::vector<std::vector<float>>{
        {5.1, 4.9, 4.7, 4.6},
        {3.5, 3.0, 3.2, 3.1},
        {1.4, 1.4, 1.3, 1.5},
        {0.2, 0.2, 0.2, 0.2}
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
    REQUIRE(arff.getX().size() == 9);
    for (int i = 0; i < 9; ++i) {
        REQUIRE(arff.getX()[i].size() == 214);
    }
    std::vector<std::vector<float>> expected = {
        {1.51793, 1.51643, 1.51793, 1.51299},
        {12.79, 12.16, 13.21, 14.4 },
        {3.5, 3.52, 3.48, 1.74},
        {1.12, 1.35, 1.41, 1.54}
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
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j)
            REQUIRE(arff.getX()[i][j] == Catch::Approx(expected[i][j]));
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
    auto states = arff.getStates();
    auto numeric = arff.getNumericAttributes();
    auto attributes = arff.getAttributes();
    for (size_t i = 0; i < numeric.size(); ++i) {
        auto feature = attributes.at(i).first;
        auto state = states.at(feature);
        if (!numeric.at(i)) {
            std::cout << feature << ": ";
            for (const auto& s : state) {
                std::cout << s << ", ";
            }
            std::cout << std::endl;
        } else {
            std::cout << feature << " size: " << state.size() << std::endl;
        }
    }
}

