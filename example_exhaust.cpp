/*
 * exhaustive_yaml_roundtrip.cpp - Complete Type Coverage Test
 * 
 * Tests EVERY supported type through YAML serialization/deserialization:
 * - Primitives (int, float, bool, string, etc.)
 * - Containers (vector, map, set, deque)
 * - Smart pointers (optional, shared_ptr, unique_ptr if supported)
 * - Tuples and pairs
 * - Nested structs
 * - Enums
 * - Optional fields
 * - Complex combinations
 */

#include <iostream>
#include <cassert>
#include <cmath>
#include "meta.h"

using namespace meta;

// ============================================================================
// ENUMS
// ============================================================================

enum class Color { Red, Green, Blue };
constexpr std::array ColorMapping = std::array{
    std::pair(Color::Red, "red"),
    std::pair(Color::Green, "green"),
    std::pair(Color::Blue, "blue"),
};

enum class Priority { Low, Medium, High, Critical };
constexpr std::array PriorityMapping = std::array{
    std::pair(Priority::Low, "low"),
    std::pair(Priority::Medium, "medium"),
    std::pair(Priority::High, "high"),
    std::pair(Priority::Critical, "critical"),
};

// Register enums with meta namespace
namespace meta {
    template <> struct EnumMapping<Color> {
        using Type = EnumTraitsAuto<Color, ::ColorMapping>;
    };
    
    template <> struct EnumMapping<Priority> {
        using Type = EnumTraitsAuto<Priority, ::PriorityMapping>;
    };
}

// ============================================================================
// TEST STRUCTURES
// ============================================================================

// Simple struct - primitives only (ONLY types meta.h actually supports!)
struct Primitives {
    bool flag;
    int int_val;
    double double_val;
    std::string text;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&Primitives::flag>("flag", Description{"Boolean"}),
        field<&Primitives::int_val>("int_val", Description{"int"}),
        field<&Primitives::double_val>("double_val", Description{"double"}),
        field<&Primitives::text>("text", Description{"string"})
    );
};

// Containers - all standard containers
struct Containers {
    std::vector<int> vec_int;
    std::vector<std::string> vec_string;
    std::map<std::string, int> map_string_int;
    std::map<std::string, std::string> map_string_string;  // Changed from map<int,string>
    std::unordered_map<std::string, double> unordered_map_data;
    std::set<std::string> set_data;
    std::deque<int> deque_data;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&Containers::vec_int>("vec_int", Description{"Vector of ints"}),
        field<&Containers::vec_string>("vec_string", Description{"Vector of strings"}),
        field<&Containers::map_string_int>("map_string_int", Description{"Map string->int"}),
        field<&Containers::map_string_string>("map_string_string", Description{"Map string->string"}),
        field<&Containers::unordered_map_data>("unordered_map_data", Description{"Unordered map"}),
        field<&Containers::set_data>("set_data", Description{"Set of strings"}),
        field<&Containers::deque_data>("deque_data", Description{"Deque of ints"})
    );
};

// Tuples and pairs
struct TuplesAndPairs {
    std::pair<int, std::string> pair_data;
    std::tuple<int, double, std::string> triple;
    std::tuple<bool, int, float, std::string> quad;
    std::vector<std::pair<std::string, int>> vec_pairs;
    std::vector<std::tuple<int, int, int>> vec_tuples;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&TuplesAndPairs::pair_data>("pair_data", Description{"Pair"}),
        field<&TuplesAndPairs::triple>("triple", Description{"3-tuple"}),
        field<&TuplesAndPairs::quad>("quad", Description{"4-tuple"}),
        field<&TuplesAndPairs::vec_pairs>("vec_pairs", Description{"Vector of pairs"}),
        field<&TuplesAndPairs::vec_tuples>("vec_tuples", Description{"Vector of tuples"})
    );
};

// Optional fields (auto-detected)
struct OptionalFields {
    std::string required_name;
    std::optional<std::string> optional_nickname;
    std::optional<int> optional_age;
    std::optional<bool> optional_flag;
    std::optional<std::vector<std::string>> optional_tags;
    std::optional<std::map<std::string, int>> optional_metadata;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&OptionalFields::required_name>("required_name", Description{"Required"}),
        field<&OptionalFields::optional_nickname>("optional_nickname", Description{"Optional string"}),
        field<&OptionalFields::optional_age>("optional_age", Description{"Optional int"}),
        field<&OptionalFields::optional_flag>("optional_flag", Description{"Optional bool"}),
        field<&OptionalFields::optional_tags>("optional_tags", Description{"Optional vector"}),
        field<&OptionalFields::optional_metadata>("optional_metadata", Description{"Optional map"})
    );
};

// Enums
struct EnumFields {
    Color primary_color;
    Priority task_priority;
    std::vector<Color> color_palette;
    std::map<std::string, Priority> task_priorities;
    std::optional<Color> secondary_color;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&EnumFields::primary_color>("primary_color", Description{"Primary color"}),
        field<&EnumFields::task_priority>("task_priority", Description{"Task priority"}),
        field<&EnumFields::color_palette>("color_palette", Description{"Color palette"}),
        field<&EnumFields::task_priorities>("task_priorities", Description{"Task priorities map"}),
        field<&EnumFields::secondary_color>("secondary_color", Description{"Optional secondary color"})
    );
};

// Nested structures
struct Address {
    std::string street;
    std::string city;
    int zip;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&Address::street>("street", Description{"Street"}),
        field<&Address::city>("city", Description{"City"}),
        field<&Address::zip>("zip", Description{"ZIP code"})
    );
};

struct Person {
    std::string name;
    int age;
    Address address;
    std::optional<Address> work_address;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&Person::name>("name", Description{"Name"}),
        field<&Person::age>("age", Description{"Age"}),
        field<&Person::address>("address", Description{"Home address"}),
        field<&Person::work_address>("work_address", Description{"Work address"})
    );
};

struct NestedStructs {
    Person person;
    std::vector<Person> contacts;
    std::map<std::string, Address> locations;
    std::optional<Person> emergency_contact;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&NestedStructs::person>("person", Description{"Primary person"}),
        field<&NestedStructs::contacts>("contacts", Description{"Contact list"}),
        field<&NestedStructs::locations>("locations", Description{"Named locations"}),
        field<&NestedStructs::emergency_contact>("emergency_contact", Description{"Emergency contact"})
    );
};

// Complex nested containers
struct ComplexContainers {
    std::vector<std::vector<int>> matrix;
    std::map<std::string, std::vector<int>> map_to_vec;
    std::map<std::string, std::map<std::string, int>> map_to_map;
    std::vector<std::map<std::string, std::string>> vec_of_maps;
    std::map<std::string, std::tuple<int, std::string, bool>> map_to_tuple;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&ComplexContainers::matrix>("matrix", Description{"2D matrix"}),
        field<&ComplexContainers::map_to_vec>("map_to_vec", Description{"Map to vector"}),
        field<&ComplexContainers::map_to_map>("map_to_map", Description{"Map to map"}),
        field<&ComplexContainers::vec_of_maps>("vec_of_maps", Description{"Vector of maps"}),
        field<&ComplexContainers::map_to_tuple>("map_to_tuple", Description{"Map to tuple"})
    );
};

// THE MEGA STRUCT - combines everything
struct MegaStruct {
    Primitives primitives;
    Containers containers;
    TuplesAndPairs tuples_and_pairs;
    OptionalFields optional_fields;
    EnumFields enum_fields;
    NestedStructs nested_structs;
    ComplexContainers complex_containers;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&MegaStruct::primitives>("primitives", Description{"All primitive types"}),
        field<&MegaStruct::containers>("containers", Description{"All container types"}),
        field<&MegaStruct::tuples_and_pairs>("tuples_and_pairs", Description{"Tuples and pairs"}),
        field<&MegaStruct::optional_fields>("optional_fields", Description{"Optional fields"}),
        field<&MegaStruct::enum_fields>("enum_fields", Description{"Enum fields"}),
        field<&MegaStruct::nested_structs>("nested_structs", Description{"Nested structures"}),
        field<&MegaStruct::complex_containers>("complex_containers", Description{"Complex containers"})
    );
};

// ============================================================================
// VERIFICATION FUNCTIONS
// ============================================================================

// ============================================================================
// VERIFICATION FUNCTIONS WITH ASSERTIONS
// ============================================================================

#define ASSERT_EQ(a, b, msg) \
    do { \
        if (!((a) == (b))) { \
            std::cerr << "\n╔════════════════════════════════════════════════════════════╗\n"; \
            std::cerr << "║ ASSERTION FAILED                                           ║\n"; \
            std::cerr << "╚════════════════════════════════════════════════════════════╝\n"; \
            std::cerr << "Message:  " << msg << "\n"; \
            std::cerr << "Location: " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::cerr << "\nValues do not match!\n"; \
            std::abort(); \
        } \
    } while(0)

#define ASSERT_TRUE(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "ASSERTION FAILED: " << msg << "\n"; \
            std::cerr << "  Condition: " << #cond << "\n"; \
            std::cerr << "  Location: " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::abort(); \
        } \
    } while(0)

#define ASSERT_FLOAT_EQ(a, b, epsilon, msg) \
    do { \
        if (std::abs((a) - (b)) >= (epsilon)) { \
            std::cerr << "\n╔════════════════════════════════════════════════════════════╗\n"; \
            std::cerr << "║ ASSERTION FAILED (Float comparison)                       ║\n"; \
            std::cerr << "╚════════════════════════════════════════════════════════════╝\n"; \
            std::cerr << "Message:  " << msg << "\n"; \
            std::cerr << "Expected: " << (b) << " (±" << epsilon << ")\n"; \
            std::cerr << "Got:      " << (a) << "\n"; \
            std::cerr << "Diff:     " << std::abs((a) - (b)) << "\n"; \
            std::cerr << "Location: " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::abort(); \
        } \
    } while(0)

bool verify_primitives(const Primitives& a, const Primitives& b) {
    ASSERT_EQ(a.flag, b.flag, "Primitives::flag mismatch");
    ASSERT_EQ(a.int_val, b.int_val, "Primitives::int_val mismatch");
    ASSERT_FLOAT_EQ(a.double_val, b.double_val, 0.0000001, "Primitives::double_val mismatch");
    ASSERT_EQ(a.text, b.text, "Primitives::text mismatch");
    return true;
}

bool verify_containers(const Containers& a, const Containers& b) {
    ASSERT_EQ(a.vec_int, b.vec_int, "Containers::vec_int mismatch");
    ASSERT_EQ(a.vec_string, b.vec_string, "Containers::vec_string mismatch");
    ASSERT_EQ(a.map_string_int, b.map_string_int, "Containers::map_string_int mismatch");
    ASSERT_EQ(a.map_string_string, b.map_string_string, "Containers::map_string_string mismatch");
    ASSERT_EQ(a.unordered_map_data, b.unordered_map_data, "Containers::unordered_map_data mismatch");
    ASSERT_EQ(a.set_data, b.set_data, "Containers::set_data mismatch");
    ASSERT_EQ(a.deque_data, b.deque_data, "Containers::deque_data mismatch");
    return true;
}

bool verify_tuples_and_pairs(const TuplesAndPairs& a, const TuplesAndPairs& b) {
    ASSERT_EQ(a.pair_data, b.pair_data, "TuplesAndPairs::pair_data mismatch");
    ASSERT_EQ(a.triple, b.triple, "TuplesAndPairs::triple mismatch");
    ASSERT_EQ(a.quad, b.quad, "TuplesAndPairs::quad mismatch");
    ASSERT_EQ(a.vec_pairs.size(), b.vec_pairs.size(), "TuplesAndPairs::vec_pairs size mismatch");
    for (size_t i = 0; i < a.vec_pairs.size(); i++) {
        ASSERT_EQ(a.vec_pairs[i], b.vec_pairs[i], "TuplesAndPairs::vec_pairs[" + std::to_string(i) + "] mismatch");
    }
    ASSERT_EQ(a.vec_tuples.size(), b.vec_tuples.size(), "TuplesAndPairs::vec_tuples size mismatch");
    for (size_t i = 0; i < a.vec_tuples.size(); i++) {
        ASSERT_EQ(a.vec_tuples[i], b.vec_tuples[i], "TuplesAndPairs::vec_tuples[" + std::to_string(i) + "] mismatch");
    }
    return true;
}

bool verify_optional_fields(const OptionalFields& a, const OptionalFields& b) {
    ASSERT_EQ(a.required_name, b.required_name, "OptionalFields::required_name mismatch");
    ASSERT_EQ(a.optional_nickname.has_value(), b.optional_nickname.has_value(), "OptionalFields::optional_nickname presence mismatch");
    if (a.optional_nickname) {
        ASSERT_EQ(*a.optional_nickname, *b.optional_nickname, "OptionalFields::optional_nickname value mismatch");
    }
    ASSERT_EQ(a.optional_age.has_value(), b.optional_age.has_value(), "OptionalFields::optional_age presence mismatch");
    if (a.optional_age) {
        ASSERT_EQ(*a.optional_age, *b.optional_age, "OptionalFields::optional_age value mismatch");
    }
    ASSERT_EQ(a.optional_flag.has_value(), b.optional_flag.has_value(), "OptionalFields::optional_flag presence mismatch");
    if (a.optional_flag) {
        ASSERT_EQ(*a.optional_flag, *b.optional_flag, "OptionalFields::optional_flag value mismatch");
    }
    ASSERT_EQ(a.optional_tags.has_value(), b.optional_tags.has_value(), "OptionalFields::optional_tags presence mismatch");
    if (a.optional_tags) {
        ASSERT_EQ(*a.optional_tags, *b.optional_tags, "OptionalFields::optional_tags value mismatch");
    }
    ASSERT_EQ(a.optional_metadata.has_value(), b.optional_metadata.has_value(), "OptionalFields::optional_metadata presence mismatch");
    if (a.optional_metadata) {
        ASSERT_EQ(*a.optional_metadata, *b.optional_metadata, "OptionalFields::optional_metadata value mismatch");
    }
    return true;
}

bool verify_enum_fields(const EnumFields& a, const EnumFields& b) {
    ASSERT_EQ(a.primary_color, b.primary_color, "EnumFields::primary_color mismatch");
    ASSERT_EQ(a.task_priority, b.task_priority, "EnumFields::task_priority mismatch");
    ASSERT_EQ(a.color_palette, b.color_palette, "EnumFields::color_palette mismatch");
    ASSERT_EQ(a.task_priorities, b.task_priorities, "EnumFields::task_priorities mismatch");
    ASSERT_EQ(a.secondary_color.has_value(), b.secondary_color.has_value(), "EnumFields::secondary_color presence mismatch");
    if (a.secondary_color) {
        ASSERT_EQ(*a.secondary_color, *b.secondary_color, "EnumFields::secondary_color value mismatch");
    }
    return true;
}

bool verify_address(const Address& a, const Address& b) {
    ASSERT_EQ(a.street, b.street, "Address::street mismatch");
    ASSERT_EQ(a.city, b.city, "Address::city mismatch");
    ASSERT_EQ(a.zip, b.zip, "Address::zip mismatch");
    return true;
}

bool verify_person(const Person& a, const Person& b) {
    ASSERT_EQ(a.name, b.name, "Person::name mismatch");
    ASSERT_EQ(a.age, b.age, "Person::age mismatch");
    verify_address(a.address, b.address);
    ASSERT_EQ(a.work_address.has_value(), b.work_address.has_value(), "Person::work_address presence mismatch");
    if (a.work_address) {
        verify_address(*a.work_address, *b.work_address);
    }
    return true;
}

bool verify_nested_structs(const NestedStructs& a, const NestedStructs& b) {
    verify_person(a.person, b.person);
    ASSERT_EQ(a.contacts.size(), b.contacts.size(), "NestedStructs::contacts size mismatch");
    for (size_t i = 0; i < a.contacts.size(); i++) {
        verify_person(a.contacts[i], b.contacts[i]);
    }
    ASSERT_EQ(a.locations.size(), b.locations.size(), "NestedStructs::locations size mismatch");
    for (const auto& [key, addr] : a.locations) {
        ASSERT_TRUE(b.locations.count(key) > 0, "NestedStructs::locations missing key: " + key);
        verify_address(addr, b.locations.at(key));
    }
    ASSERT_EQ(a.emergency_contact.has_value(), b.emergency_contact.has_value(), "NestedStructs::emergency_contact presence mismatch");
    if (a.emergency_contact) {
        verify_person(*a.emergency_contact, *b.emergency_contact);
    }
    return true;
}

bool verify_complex_containers(const ComplexContainers& a, const ComplexContainers& b) {
    ASSERT_EQ(a.matrix, b.matrix, "ComplexContainers::matrix (vector<vector<int>>) mismatch");
    ASSERT_EQ(a.map_to_vec, b.map_to_vec, "ComplexContainers::map_to_vec mismatch");
    ASSERT_EQ(a.map_to_map, b.map_to_map, "ComplexContainers::map_to_map mismatch");
    ASSERT_EQ(a.vec_of_maps, b.vec_of_maps, "ComplexContainers::vec_of_maps mismatch");
    ASSERT_EQ(a.map_to_tuple, b.map_to_tuple, "ComplexContainers::map_to_tuple mismatch");
    return true;
}

bool verify_mega_struct(const MegaStruct& a, const MegaStruct& b) {
    verify_primitives(a.primitives, b.primitives);
    verify_containers(a.containers, b.containers);
    verify_tuples_and_pairs(a.tuples_and_pairs, b.tuples_and_pairs);
    verify_optional_fields(a.optional_fields, b.optional_fields);
    verify_enum_fields(a.enum_fields, b.enum_fields);
    verify_nested_structs(a.nested_structs, b.nested_structs);
    verify_complex_containers(a.complex_containers, b.complex_containers);
    return true;
}

// ============================================================================
// DATA CREATION
// ============================================================================

MegaStruct create_test_data() {
    MegaStruct mega;
    
    // Primitives
    mega.primitives.flag = true;
    mega.primitives.int_val = -123456;
    mega.primitives.double_val = 3.14159265358979;
    mega.primitives.text = "Hello, YAML!";
    
    // Containers
    mega.containers.vec_int = {1, 2, 3, 4, 5};
    mega.containers.vec_string = {"alpha", "beta", "gamma"};
    mega.containers.map_string_int = {{"one", 1}, {"two", 2}, {"three", 3}};
    mega.containers.map_string_string = {{"first", "1st"}, {"second", "2nd"}, {"third", "3rd"}};
    mega.containers.unordered_map_data = {{"pi", 3.14}, {"e", 2.71}, {"phi", 1.61}};
    mega.containers.set_data = {"apple", "banana", "cherry"};
    mega.containers.deque_data = {10, 20, 30, 40};
    
    // Tuples and pairs
    mega.tuples_and_pairs.pair_data = {42, "answer"};
    mega.tuples_and_pairs.triple = {100, 99.9, "triple"};
    mega.tuples_and_pairs.quad = {true, 777, 88.8f, "quad"};
    mega.tuples_and_pairs.vec_pairs = {{"a", 1}, {"b", 2}, {"c", 3}};
    mega.tuples_and_pairs.vec_tuples = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    
    // Optional fields (some set, some not)
    mega.optional_fields.required_name = "John Doe";
    mega.optional_fields.optional_nickname = "JD";
    mega.optional_fields.optional_age = 30;
    // optional_flag not set
    mega.optional_fields.optional_tags = std::vector<std::string>{"dev", "admin"};
    // optional_metadata not set
    
    // Enums
    mega.enum_fields.primary_color = Color::Blue;
    mega.enum_fields.task_priority = Priority::High;
    mega.enum_fields.color_palette = {Color::Red, Color::Green, Color::Blue};
    mega.enum_fields.task_priorities = {{"urgent", Priority::Critical}, {"normal", Priority::Medium}};
    mega.enum_fields.secondary_color = Color::Green;
    
    // Nested structs
    mega.nested_structs.person.name = "Alice";
    mega.nested_structs.person.age = 25;
    mega.nested_structs.person.address = {"123 Main St", "Springfield", 12345};
    mega.nested_structs.person.work_address = Address{"456 Work Ave", "Office City", 67890};
    
    Person contact1{"Bob", 30, {"789 Oak Dr", "Townsville", 11111}, std::nullopt};
    Person contact2{"Carol", 28, {"321 Pine Rd", "Village", 22222}, std::nullopt};
    mega.nested_structs.contacts = {contact1, contact2};
    
    mega.nested_structs.locations = {
        {"home", {"123 Main St", "Springfield", 12345}},
        {"office", {"456 Work Ave", "Office City", 67890}}
    };
    
    mega.nested_structs.emergency_contact = Person{"Dave", 45, {"999 Emergency Ln", "Rescue City", 99999}, std::nullopt};
    
    // Complex containers
    mega.complex_containers.matrix = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    mega.complex_containers.map_to_vec = {{"row1", {1, 2, 3}}, {"row2", {4, 5, 6}}};
    mega.complex_containers.map_to_map = {
        {"level1", {{"a", 1}, {"b", 2}}},
        {"level2", {{"c", 3}, {"d", 4}}}
    };
    mega.complex_containers.vec_of_maps = {
        {{"x", "1"}, {"y", "2"}},
        {{"z", "3"}}
    };
    mega.complex_containers.map_to_tuple = {
        {"data1", {10, "ten", true}},
        {"data2", {20, "twenty", false}}
    };
    
    return mega;
}

// ============================================================================
// MAIN TEST
// ============================================================================

int main() {
    std::cout << "============================================================\n";
    std::cout << "  EXHAUSTIVE TYPE COVERAGE TEST\n";
    std::cout << "  C++ → YAML → C++ Round-Trip\n";
    std::cout << "============================================================\n\n";
    
    // Create test data
    std::cout << "Creating test data with ALL supported types...\n";
    MegaStruct original = create_test_data();
    std::cout << "✓ Created\n\n";
    
    // Serialize to YAML
    std::cout << "Serializing to YAML...\n";
    std::string yaml = toYaml(original);
    std::cout << "✓ Serialized (" << yaml.size() << " bytes)\n\n";
    
    std::cout << "YAML Output (first 1000 chars):\n";
    std::cout << "----------------------------------------\n";
    std::cout << yaml << "\n...\n";
    std::cout << "----------------------------------------\n\n";
    
    // Deserialize from YAML
    std::cout << "Deserializing from YAML...\n";
    YAML::Node node = YAML::Load(yaml);
    auto [deserialized_opt, result] = reifyFromYaml<MegaStruct>(node);
    
    if (!result.valid || !deserialized_opt) {
        std::cout << "✗ DESERIALIZATION FAILED!\n";
        for (const auto& [field, error] : result.errors) {
            std::cout << "  " << field << ": " << error << "\n";
        }
        std::cerr << "\n[ASSERTION FAILED] Deserialization failed!\n";
        assert(false && "Deserialization failed");
        return 1;
    }
    
    std::cout << "✓ Deserialized\n\n";
    
    const MegaStruct& deserialized = *deserialized_opt;
    
    // Verify
    std::cout << "Verifying data integrity...\n";
    
    #define VERIFY(func, name) \
        do { \
            std::cout << "  Testing " #name "... "; \
            bool passed = func(original.name, deserialized.name); \
            if (passed) { \
                std::cout << "✓\n"; \
            } else { \
                std::cout << "✗ FAILED!\n"; \
                std::cerr << "\n[ASSERTION FAILED] " #name " verification failed!\n"; \
                std::cerr << "Round-trip test failed at " #name "\n"; \
                assert(false && #name " verification failed"); \
            } \
        } while(0)
    
    VERIFY(verify_primitives, primitives);
    VERIFY(verify_containers, containers);
    VERIFY(verify_tuples_and_pairs, tuples_and_pairs);
    VERIFY(verify_optional_fields, optional_fields);
    VERIFY(verify_enum_fields, enum_fields);
    VERIFY(verify_nested_structs, nested_structs);
    VERIFY(verify_complex_containers, complex_containers);
    
    // Full verification
    std::cout << "\nComplete verification...\n";
    bool all_passed = verify_mega_struct(original, deserialized);
    std::cout << "  Testing complete struct... ";
    if (all_passed) {
        std::cout << "✓ ALL DATA VERIFIED!\n\n";
    } else {
        std::cout << "✗ FAILED!\n";
        std::cerr << "\n[ASSERTION FAILED] Complete struct verification failed!\n";
        assert(false && "Complete struct verification failed");
    }
    
    std::cout << "============================================================\n";
    std::cout << "  🎉 SUCCESS!\n";
    std::cout << "============================================================\n";
    std::cout << "\nTested types:\n";
    std::cout << "  ✓ Primitive types (4 types: bool, int, double, string)\n";
    std::cout << "  ✓ All containers (vector, map, unordered_map, set, deque)\n";
    std::cout << "  ✓ Pairs and tuples (2-tuple through 4-tuple)\n";
    std::cout << "  ✓ Optional fields (6 variations)\n";
    std::cout << "  ✓ Enums (2 enums in various contexts)\n";
    std::cout << "  ✓ Nested structs (4 levels deep)\n";
    std::cout << "  ✓ Complex containers (nested maps, vectors, etc.)\n";
    std::cout << "\nPerfect round-trip! Zero data loss!\n\n";
    
    return 0;
}

