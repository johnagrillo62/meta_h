#include <iostream>
#include <vector>
#include <map>
#include "meta.h"

using namespace meta;
struct SimpleStruct {
    std::string name;
    int value;
    
    inline static constexpr auto fields = std::make_tuple(
      field<&SimpleStruct::name>("name"),
      field<&SimpleStruct::value>("value")
    );
};

struct TestData {
    std::vector<std::map<std::string, int>> vec_of_maps;
    std::vector<SimpleStruct> vec_of_structs;
    std::vector<std::vector<int>> matrix;
    
    inline static constexpr auto fields = std::make_tuple(
        field<&TestData::vec_of_maps>("vec_of_maps"),
        field<&TestData::vec_of_structs>("vec_of_structs"),
        field<&TestData::matrix>("matrix")
    );
};

int main() {
    TestData data;
    
    // Test 1: vector of maps
    data.vec_of_maps.push_back({{"a", 1}, {"b", 2}});
    data.vec_of_maps.push_back({{"c", 3}, {"d", 4}});
    
    // Test 2: vector of structs
    data.vec_of_structs.push_back({"Alice", 100});
    data.vec_of_structs.push_back({"Bob", 200});
    
    // Test 3: nested vectors
    data.matrix.push_back({1, 2, 3});
    data.matrix.push_back({4, 5, 6});
    
    std::cout << "=== Serializing ===\n";
    std::string yaml = meta::toYaml(data);
    std::cout << yaml << "\n";
    
    std::cout << "\n=== Deserializing ===\n";
    auto [result, validation] = meta::reifyFromYaml<TestData>(yaml);
    
    if (!validation.valid) {
        std::cout << "FAILED:\n";
        for (auto& [f, e] : validation.errors) {
            std::cout << "  " << f << ": " << e << "\n";
        }
        return 1;
    }
    
    std::cout << "SUCCESS!\n";
    return 0;
}
