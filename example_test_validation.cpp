// test_validation.cpp - Quick test of missing/extra field validation
#include "meta.h"
#include <iostream>

struct User {
    int id;
    std::string name;
    std::string email;
    
    static constexpr auto fields = std::make_tuple(
        meta::field<&User::id>("id"),
        meta::field<&User::name>("name"),
        meta::field<&User::email>("email")
    );
};

int main() {
    std::cout << "Testing Field Validation\n";
    std::cout << "========================\n\n";
    
    // Test 1: Extra field
    std::cout << "Test 1: Extra field (typo)\n";
    std::string yaml1 = R"(
id: 1
name: Alice
emial: alice@test.com
email: real@test.com
)";
    
    auto [u1, r1] = meta::reifyFromYaml<User>(yaml1);
    if (!r1.valid) {
        std::cout << "✓ Caught errors:\n";
        for (const auto& [field, msg] : r1.errors) {
            std::cout << "  - " << field << ": " << msg << "\n";
        }
    } else {
        std::cout << "✗ Should have failed!\n";
    }
    
    // Test 2: Missing required field
    std::cout << "\nTest 2: Missing required field\n";
    std::string yaml2 = R"(
id: 2
name: Bob
)";
    
    auto [u2, r2] = meta::reifyFromYaml<User>(yaml2);
    if (!r2.valid) {
        std::cout << "✓ Caught errors:\n";
        for (const auto& [field, msg] : r2.errors) {
            std::cout << "  - " << field << ": " << msg << "\n";
        }
    } else {
        std::cout << "✗ Should have failed!\n";
    }
    
    // Test 3: Multiple errors
    std::cout << "\nTest 3: Multiple errors\n";
    std::string yaml3 = R"(
id: 3
emial: typo@test.com
extra_field: bad
another_bad: 123
)";
    
    auto [u3, r3] = meta::reifyFromYaml<User>(yaml3);
    if (!r3.valid) {
        std::cout << "✓ Caught " << r3.errors.size() << " errors:\n";
        for (const auto& [field, msg] : r3.errors) {
            std::cout << "  - " << field << ": " << msg << "\n";
        }
    } else {
        std::cout << "✗ Should have failed!\n";
    }
    
    // Test 4: Valid YAML
    std::cout << "\nTest 4: Valid YAML (should succeed)\n";
    std::string yaml4 = R"(
id: 4
name: Charlie
email: charlie@test.com
)";
    
    auto [u4, r4] = meta::reifyFromYaml<User>(yaml4);
    if (r4.valid) {
        std::cout << "✓ Parsed successfully\n";
        std::cout << "  ID: " << u4->id << "\n";
        std::cout << "  Name: " << u4->name << "\n";
        std::cout << "  Email: " << u4->email << "\n";
    } else {
        std::cout << "✗ Should have succeeded!\n";
        for (const auto& [field, msg] : r4.errors) {
            std::cout << "  - " << field << ": " << msg << "\n";
        }
    }
    
    std::cout << "\n✓ All tests complete!\n";
    return 0;
}

