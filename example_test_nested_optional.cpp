// test_nested_optional.cpp - Test nested structs with optional fields
#include <iostream>
#include <optional>
#include "meta.h"

// ============================================================================
// Nested structs with optional fields
// ============================================================================

struct Address {
    std::string street;
    std::string city;
    std::optional<std::string> state;
    std::string country;
    std::optional<int> zip_code;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        meta::field<&Address::street>("street"),
        meta::field<&Address::city>("city"),
        meta::field<&Address::state>("state"),
        meta::field<&Address::country>("country"),
        meta::field<&Address::zip_code>("zip_code")
    );
};

struct Company {
    std::string name;
    std::optional<Address> headquarters;
    std::vector<Address> offices;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        meta::field<&Company::name>("name"),
        meta::field<&Company::headquarters>("headquarters"),
        meta::field<&Company::offices>("offices")
    );
};

struct Employee {
    int id;
    std::string name;
    std::optional<std::string> middle_name;
    Address home_address;
    std::optional<Address> mailing_address;
    std::optional<Company> employer;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        meta::field<&Employee::id>("id"),
        meta::field<&Employee::name>("name"),
        meta::field<&Employee::middle_name>("middle_name"),
        meta::field<&Employee::home_address>("home_address"),
        meta::field<&Employee::mailing_address>("mailing_address"),
        meta::field<&Employee::employer>("employer")
    );
};

// ============================================================================
// Good YAML examples
// ============================================================================

const char* good_yaml_1 = R"(
id: 1
name: Alice Johnson
home_address:
  street: 123 Main St
  city: Boston
  state: MA
  country: USA
  zip_code: 02101
)";

const char* good_yaml_2 = R"(
id: 2
name: Bob Smith
middle_name: William
home_address:
  street: 456 Oak Ave
  city: Seattle
  country: USA
mailing_address:
  street: PO Box 789
  city: Seattle
  state: WA
  country: USA
  zip_code: 98101
)";

const char* good_yaml_3 = R"(
id: 3
name: Charlie Brown
home_address:
  street: 789 Elm St
  city: London
  country: UK
employer:
  name: Tech Corp
  headquarters:
    street: 100 Innovation Dr
    city: San Francisco
    state: CA
    country: USA
    zip_code: 94105
  offices:
    - street: 200 King St
      city: Toronto
      country: Canada
    - street: 300 Queen St
      city: Sydney
      country: Australia
)";

// ============================================================================
// Bad YAML examples
// ============================================================================

const char* bad_yaml_1 = R"(
id: 1
name: Alice
)";

const char* bad_yaml_2 = R"(
id: 2
name: Bob
home_address: "123 Main St"
)";

const char* bad_yaml_3 = R"(
id: 3
name: Charlie
home_address:
  street: 123 Main St
  country: USA
)";

const char* bad_yaml_4 = R"(
id: 4
name: Dave
middle_name: 123
home_address:
  street: 456 Oak
  city: NYC
  country: USA
)";

const char* bad_yaml_5 = R"(
id: 5
name: Eve
home_address:
  - street: 789 Elm
    city: LA
    country: USA
)";

// ============================================================================
// Test runner
// ============================================================================

void test_yaml(const char* description, const char* yaml_str, bool should_succeed) {
    std::cout << "\n" << description << "\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << yaml_str << "\n";
    
    try {
        auto [emp_opt, validation] = meta::reifyFromYaml<Employee>(yaml_str);
        
        if (!emp_opt.has_value()) {
            if (!should_succeed) {
                std::cout << "✓ PASSED: Validation failed as expected\n";
            } else {
                std::cout << "✗ FAILED: Should have succeeded\n";
            }
            std::cout << "  Errors:\n";
            for (const auto& [field, msg] : validation.errors) {
                std::cout << "    - " << field << ": " << msg << "\n";
            }
            return;
        }
        
        Employee& emp = *emp_opt;
        
        if (should_succeed) {
            std::cout << "✓ PASSED: Parsed successfully\n";
            std::cout << "  ID: " << emp.id << "\n";
            std::cout << "  Name: " << emp.name << "\n";
            if (emp.middle_name) {
                std::cout << "  Middle: " << *emp.middle_name << "\n";
            }
            std::cout << "  Address: " << emp.home_address.street 
                      << ", " << emp.home_address.city << "\n";
            if (emp.mailing_address) {
                std::cout << "  Mailing: " << emp.mailing_address->city << "\n";
            }
            if (emp.employer) {
                std::cout << "  Employer: " << emp.employer->name << "\n";
                if (emp.employer->headquarters) {
                    std::cout << "    HQ: " << emp.employer->headquarters->city << "\n";
                }
                std::cout << "    Offices: " << emp.employer->offices.size() << "\n";
            }
        } else {
            std::cout << "✗ FAILED: Should have thrown error\n";
        }
    } catch (const std::exception& e) {
        if (!should_succeed) {
            std::cout << "✓ PASSED: Caught error as expected\n";
        } else {
            std::cout << "✗ FAILED: Should not have thrown\n";
        }
        std::cout << "  Error: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "NESTED OPTIONAL STRUCT YAML TESTS\n";
    std::cout << "==================================\n";
    
    test_yaml("Good 1: Minimal required fields", good_yaml_1, true);
    test_yaml("Good 2: With optional fields", good_yaml_2, true);
    test_yaml("Good 3: Deeply nested with arrays", good_yaml_3, true);
    
    test_yaml("Bad 1: Missing required nested struct", bad_yaml_1, false);
    test_yaml("Bad 2: String instead of nested object", bad_yaml_2, false);
    test_yaml("Bad 3: Missing required field in nested", bad_yaml_3, false);
    test_yaml("Bad 4: Wrong type for optional field", bad_yaml_4, false);
    test_yaml("Bad 5: Array instead of single object", bad_yaml_5, false);
    
    return 0;
}

