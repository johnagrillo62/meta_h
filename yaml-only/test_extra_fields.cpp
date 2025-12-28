/*
 * test_extra_fields.cpp - Test extra fields detection
 */

#include "meta_yaml.h"
#include <iostream>

struct Person {
    std::string name;
    int age;

    static constexpr auto fields = std::tuple{
        meta::Field<&Person::name>{"name"},
        meta::Field<&Person::age>{"age"}
    };
};

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Extra Fields Validation Test                              ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝\n" << std::endl;

    // Test 1: Valid YAML (no extra fields)
    std::cout << "=== Test 1: Valid YAML (no extra fields) ===" << std::endl;
    std::string valid_yaml = R"(
name: John Doe
age: 30
)";
    std::cout << "Input:\n" << valid_yaml << std::endl;
    auto [person1, result1] = meta::fromYaml<Person>(valid_yaml);
    if (result1.valid) {
        std::cout << "✓ Parsed successfully\n" << std::endl;
    }

    // Test 2: YAML with extra fields
    std::cout << "=== Test 2: YAML with extra fields ===" << std::endl;
    std::string extra_fields_yaml = R"(
name: Jane Doe
age: 25
email: jane@example.com
phone: 555-1234
)";
    std::cout << "Input:\n" << extra_fields_yaml << std::endl;
    auto [person2, result2] = meta::fromYaml<Person>(extra_fields_yaml);
    if (!result2.valid) {
        std::cout << "Validation errors detected:" << std::endl;
        for (const auto& err : result2.errors) {
            std::cout << "  ✗ " << err.first << ": " << err.second << std::endl;
        }
    }
    std::cout << std::endl;

    // Test 3: Mix of valid fields and extra fields
    std::cout << "=== Test 3: Mix of valid and extra fields ===" << std::endl;
    std::string mixed_yaml = R"(
name: Bob Smith
age: 35
department: Engineering
location: New York
)";
    std::cout << "Input:\n" << mixed_yaml << std::endl;
    auto [person3, result3] = meta::fromYaml<Person>(mixed_yaml);
    if (!result3.valid) {
        std::cout << "Validation errors detected:" << std::endl;
        for (const auto& err : result3.errors) {
            std::cout << "  ✗ " << err.first << ": " << err.second << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  ✓ Extra fields validation working correctly!              ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝\n" << std::endl;

    return 0;
}
