// massive_yaml_errors.cpp - Show ALL validation errors at once
#include <iostream>
#include <optional>
#include "meta.h"

struct Address {
    std::string street;
    std::string city;
    std::optional<std::string> state;
    std::string country;
    std::optional<int> zip_code;
    
    static constexpr auto fields = std::make_tuple(
        meta::field<&Address::street>("street"),
        meta::field<&Address::city>("city"),
        meta::field<&Address::state>("state"),
        meta::field<&Address::country>("country"),
        meta::field<&Address::zip_code>("zip_code")
    );
};

struct ContactInfo {
    std::string email;
    std::optional<std::string> phone;
    std::optional<std::string> fax;
    
    static constexpr auto fields = std::make_tuple(
        meta::field<&ContactInfo::email>("email"),
        meta::field<&ContactInfo::phone>("phone"),
        meta::field<&ContactInfo::fax>("fax")
    );
};

struct Company {
    std::string name;
    int employee_count;
    std::optional<Address> headquarters;
    std::vector<Address> offices;
    std::optional<ContactInfo> contact;
    
    static constexpr auto fields = std::make_tuple(
        meta::field<&Company::name>("name"),
        meta::field<&Company::employee_count>("employee_count"),
        meta::field<&Company::headquarters>("headquarters"),
        meta::field<&Company::offices>("offices"),
        meta::field<&Company::contact>("contact")
    );
};

struct Employee {
    int id;
    std::string name;
    std::optional<std::string> middle_name;
    int age;
    std::string department;
    Address home_address;
    std::optional<Address> mailing_address;
    std::optional<Company> employer;
    std::vector<std::string> skills;
    
    static constexpr auto fields = std::make_tuple(
        meta::field<&Employee::id>("id"),
        meta::field<&Employee::name>("name"),
        meta::field<&Employee::middle_name>("middle_name"),
        meta::field<&Employee::age>("age"),
        meta::field<&Employee::department>("department"),
        meta::field<&Employee::home_address>("home_address"),
        meta::field<&Employee::mailing_address>("mailing_address"),
        meta::field<&Employee::employer>("employer"),
        meta::field<&Employee::skills>("skills")
    );
};

// ============================================================================
// MASSIVE YAML ERROR TEST - Multiple errors in one document
// ============================================================================

const char* horrible_yaml = R"(
# This YAML has MANY errors - let's catch them all!

id: "not_a_number"              # ERROR 1: String instead of int
name: 12345                     # WARNING: Int instead of string (may auto-convert)
middle_name: [array, values]    # ERROR 2: Array instead of optional string
age: -5                         # ERROR 3: Negative age (if validated)
# department: MISSING           # ERROR 4: Missing required field

home_address: "just a string"   # ERROR 5: String instead of nested object

mailing_address:
  street: 123 Fake St
  # city: MISSING                # ERROR 6: Missing required field in nested struct
  state: CA
  country: USA
  zip_code: "not a number"      # ERROR 7: String instead of int in nested optional
  extra_field: bad              # ERROR 8: Extra field not in struct

employer:
  # name: MISSING                # ERROR 9: Missing required field in nested
  employee_count: "thousand"    # ERROR 10: String instead of int
  headquarters:
    street: 100 Main
    city: SF
    # country: MISSING           # ERROR 11: Missing required in deep nested
    zip_code: 94105
    unknown: value              # ERROR 12: Extra field in deep nested
  offices: "not an array"       # ERROR 13: String instead of vector
  contact:
    # email: MISSING             # ERROR 14: Missing required in nested
    phone: 12345                # WARNING: Int instead of string
    extra: bad                  # ERROR 15: Extra field

skills: single_string           # ERROR 16: Single value instead of array

unknown_field: should_error     # ERROR 17: Extra field at root level
another_bad: 123                # ERROR 18: Another extra field
)";

// ============================================================================
// Another test with different error types
// ============================================================================

const char* type_mismatch_yaml = R"(
id: 1
name: Bob
age: 30
department: Engineering

home_address:                   # ERROR: Array instead of object
  - street: 123 Main
    city: Boston
    country: USA

skills:                         # ERROR: Object instead of array
  cpp: true
  python: true
  java: false

employer:
  name: TechCorp
  employee_count: 500
  offices:                      # Array of objects with errors
    - street: Office 1
      city: NYC
      # country: MISSING        # Missing required in array element
    - street: Office 2
      # city: MISSING           # Missing required in another array element
      country: USA
    - "just a string"           # ERROR: String in array of objects
)";

// ============================================================================
// Nested optional with multiple errors
// ============================================================================

const char* nested_optional_errors = R"(
id: 100
name: Alice
age: 25
department: Sales

home_address:
  street: 789 Oak
  city: Seattle
  country: USA

mailing_address:                # Optional nested - but has errors inside
  street: PO Box 456
  city: null                    # ERROR: Null for required field
  country: 123                  # ERROR: Number instead of string
  state: [WA, CA]               # ERROR: Array instead of string

employer:                       # Optional nested with many errors
  name: null                    # ERROR: Null for required field
  employee_count: -100          # ERROR: Negative number
  headquarters:
    street: null                # ERROR: Null in nested
    city: 12345                 # ERROR: Number instead of string
    country: [USA, Canada]      # ERROR: Array instead of string
  offices:
    first: {}                   # ERROR: Object instead of array
  contact:
    email: null                 # ERROR: Null for required
    phone: {area: 555}          # ERROR: Object instead of string

skills:
  - skill1
  - 123                         # ERROR: Number in string array
  - null                        # ERROR: Null in string array
  - [nested, array]             # ERROR: Nested array
)";

// ============================================================================
// Test runner
// ============================================================================

void test_and_show_all_errors(const char* title, const char* yaml_str) {
    std::cout << "\n";
    std::cout << "=" << std::string(78, '=') << "=\n";
    std::cout << title << "\n";
    std::cout << "=" << std::string(78, '=') << "=\n\n";
    
    std::cout << "INPUT YAML:\n";
    std::cout << std::string(78, '-') << "\n";
    std::cout << yaml_str << "\n";
    std::cout << std::string(78, '-') << "\n\n";
    
    try {
        auto [emp_opt, validation] = meta::reifyFromYaml<Employee>(yaml_str);
        
        if (!emp_opt.has_value()) {
            std::cout << "VALIDATION FAILED ❌\n\n";
            std::cout << "Total Errors: " << validation.errors.size() << "\n\n";
            std::cout << "ERROR DETAILS:\n";
            std::cout << std::string(78, '-') << "\n";
            
            int error_num = 1;
            for (const auto& [field_path, error_msg] : validation.errors) {
                std::cout << error_num++ << ". ";
                if (field_path.empty()) {
                    std::cout << "[ROOT]: " << error_msg << "\n";
                } else {
                    std::cout << field_path << "\n";
                    std::cout << "   └─ " << error_msg << "\n";
                }
            }
            std::cout << std::string(78, '-') << "\n";
        } else {
            std::cout << "✗ UNEXPECTED: YAML parsed successfully (should have failed!)\n";
            Employee& emp = *emp_opt;
            std::cout << "Got: " << emp.name << ", ID: " << emp.id << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION THROWN ⚠️\n";
        std::cout << "Exception: " << e.what() << "\n";
    }
}

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════════════════╗
║                   COMPREHENSIVE YAML VALIDATION ERROR TEST                   ║
║                   Show ALL errors from badly-formed YAML                     ║
╚══════════════════════════════════════════════════════════════════════════════╝
)";

    test_and_show_all_errors(
        "TEST 1: MASSIVE ERROR COLLECTION (18+ different error types)",
        horrible_yaml
    );
    
    test_and_show_all_errors(
        "TEST 2: TYPE MISMATCHES (arrays vs objects, missing required in arrays)",
        type_mismatch_yaml
    );
    
    test_and_show_all_errors(
        "TEST 3: NESTED OPTIONAL ERRORS (errors inside optional nested structs)",
        nested_optional_errors
    );
    
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                              TEST COMPLETE                                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
    
    return 0;
}
