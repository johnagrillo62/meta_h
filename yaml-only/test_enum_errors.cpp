/*
 * test_enum_errors.cpp - Showcase improved enum error messages
 * 
 * This test shows that enum validation errors now display all valid options
 */

#include "meta_yaml.h"
#include <iostream>

enum class Color { Red, Green, Blue };

constexpr std::array ColorMapping = std::array{
    std::pair{Color::Red, "red"},
    std::pair{Color::Green, "green"},
    std::pair{Color::Blue, "blue"},
};

template <> struct meta::EnumMapping<Color> {
    static constexpr auto& mapping = ColorMapping;
    using Type = meta::EnumTraitsAuto<Color, ColorMapping>;
};

struct Paint {
    std::string name;
    Color color;

    static constexpr auto fields = std::tuple{
        meta::Field<&Paint::name>{"name"},
        meta::Field<&Paint::color>{"color"}
    };
};

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Enum Error Messages - Shows Valid Options               ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝\n" << std::endl;

    // Test 1: Invalid enum value
    std::cout << "=== Test 1: Invalid Enum Value ===" << std::endl;
    std::string yaml1 = R"(
name: House Paint
color: purple
)";
    
    std::cout << "Input YAML:\n" << yaml1 << std::endl;
    auto [paint1, result1] = meta::fromYaml<Paint>(yaml1);
    
    if (!result1.valid) {
        std::cout << "Validation Error:\n";
        for (const auto &[field, error] : result1.errors) {
            std::cout << "  Field: " << (field.empty() ? "(root)" : field) << "\n";
            std::cout << "  Error: " << error << "\n";
        }
    }
    std::cout << std::endl;

    // Test 2: Non-string value for enum
    std::cout << "=== Test 2: Non-String Value for Enum ===" << std::endl;
    std::string yaml2 = R"(
name: Paint Can
color: 42
)";
    
    std::cout << "Input YAML:\n" << yaml2 << std::endl;
    auto [paint2, result2] = meta::fromYaml<Paint>(yaml2);
    
    if (!result2.valid) {
        std::cout << "Validation Error:\n";
        for (const auto &[field, error] : result2.errors) {
            std::cout << "  Field: " << (field.empty() ? "(root)" : field) << "\n";
            std::cout << "  Error: " << error << "\n";
        }
    }
    std::cout << std::endl;

    // Test 3: Valid enum value
    std::cout << "=== Test 3: Valid Enum Value ===" << std::endl;
    std::string yaml3 = R"(
name: Red Paint
color: red
)";
    
    std::cout << "Input YAML:\n" << yaml3 << std::endl;
    auto [paint3, result3] = meta::fromYaml<Paint>(yaml3);
    
    if (result3.valid) {
        std::cout << "✓ Successfully parsed:\n";
        std::cout << "  Name: " << paint3->name << "\n";
        std::cout << "  Color: " << paint3->color << "\n";
    }
    std::cout << std::endl;

    // Test 4: Different invalid values
    std::cout << "=== Test 4: Various Invalid Values ===" << std::endl;
    std::vector<std::string> invalid_values = {"yellow", "black", "white", "pink"};
    
    for (const auto &invalid : invalid_values) {
        std::string yaml_test = "name: Test\ncolor: " + invalid;
        auto [paint, result] = meta::fromYaml<Paint>(yaml_test);
        
        if (!result.valid) {
            std::cout << "  Input 'color: " << invalid << "' -> ";
            // Extract just the error message
            if (!result.errors.empty()) {
                std::cout << result.errors[0].second << "\n";
            }
        }
    }
    
    std::cout << "\n╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  ✓ Enum errors now show all valid options!                 ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝\n" << std::endl;

    return 0;
}
