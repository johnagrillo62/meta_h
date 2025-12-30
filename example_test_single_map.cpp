// simple_map_test.cpp - Quick test
#include <iostream>
#include <map>
#include "meta.h"

struct Data {
    std::map<int, std::string> codes;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        meta::field<&Data::codes>("codes")
    );
};

int main() {
    // Test: Can we parse this?
    std::string yaml = R"(
codes:
  1: one
  2: two
  404: not found
)";
    
    std::cout << "Parsing:\n" << yaml << "\n";
    
    auto [data, result] = meta::reifyFromYaml<Data>(yaml);
    
    if (result.valid && data) {
        std::cout << "✓ SUCCESS!\n";
        std::cout << "Parsed values:\n";
        for (const auto& [k, v] : data->codes) {
            std::cout << "  " << k << " -> " << v << "\n";
        }
        
        // Try to serialize it back
        std::cout << "\nSerializing back to YAML:\n";
        std::cout << meta::toYaml(*data);
        
    } else {
        std::cout << "✗ FAILED\n";
        for (const auto& [field, msg] : result.errors) {
            std::cout << "  " << field << ": " << msg << "\n";
        }
    }
    
    return 0;
}
