// advanced_maps.cpp - Test various map types
#include <iostream>
#include <map>
#include <vector>
#include "meta.h"

struct AdvancedMaps {
    std::map<int, int> id_mapping;                              // int -> int
    std::map<int, std::vector<std::string>> groups;            // int -> vector
    std::map<std::string, std::map<int, std::string>> matrix;  // nested maps
    
    static constexpr auto FieldsMeta = std::make_tuple(
        meta::field<&AdvancedMaps::id_mapping>("id_mapping"),
        meta::field<&AdvancedMaps::groups>("groups"),
        meta::field<&AdvancedMaps::matrix>("matrix")
    );
};

int main() {
    std::cout << "Testing Advanced Map Types\n";
    std::cout << "===========================\n\n";
    
    // Test YAML
    std::string yaml = R"(
id_mapping:
  1: 100
  2: 200
  3: 300
groups:
  1:
    - alice
    - bob
  2:
    - charlie
    - dave
matrix:
  row1:
    1: A1
    2: A2
  row2:
    1: B1
    2: B2
)";
    
    std::cout << "Parsing complex nested maps...\n";
    auto [data, result] = meta::reifyFromYaml<AdvancedMaps>(yaml);
    
    if (result.valid && data) {
        std::cout << "\n✓ SUCCESS! All map types work!\n\n";
        
        std::cout << "id_mapping (int -> int):\n";
        for (const auto& [k, v] : data->id_mapping) {
            std::cout << "  " << k << " -> " << v << "\n";
        }
        
        std::cout << "\ngroups (int -> vector<string>):\n";
        for (const auto& [id, members] : data->groups) {
            std::cout << "  Group " << id << ": ";
            for (const auto& m : members) std::cout << m << " ";
            std::cout << "\n";
        }
        
        std::cout << "\nmatrix (string -> map<int, string>):\n";
        for (const auto& [row, cols] : data->matrix) {
            std::cout << "  " << row << ":\n";
            for (const auto& [col, val] : cols) {
                std::cout << "    [" << col << "] = " << val << "\n";
            }
        }
        
        std::cout << "\n\nSerializing back to YAML:\n";
        std::cout << meta::toYaml(*data);
        
    } else {
        std::cout << "✗ FAILED\n";
        for (const auto& [field, msg] : result.errors) {
            std::cout << "  " << field << ": " << msg << "\n";
        }
    }
    
    return 0;
}

