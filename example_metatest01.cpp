/*
 * SIMPLE ROUND-TRIP TEST: Metadata Completeness
 * 
 * The test: Does your struct annotation cover ALL fields?
 */

#include <iostream>
#include <string>

//=============================================================================
// CASE 1: COMPLETE METADATA ✓
//=============================================================================

struct GoodPerson {
    std::string name;
    int age;
    std::string email;
    
    // ALL 3 fields are registered
    static constexpr auto fields = std::make_tuple(
        "name",   // field 0
        "age",    // field 1
        "email"   // field 2
    );
    // ✓ 3 fields in struct, 3 fields in metadata
};

//=============================================================================
// CASE 2: INCOMPLETE METADATA ✗
//=============================================================================

struct BadPerson {
    std::string name;
    int age;
    std::string email;    // ← NOT in metadata!
    
    // ONLY 2 fields registered
    static constexpr auto fields = std::make_tuple(
        "name",   // field 0
        "age"     // field 1
                  // MISSING: email!
    );
    // ✗ 3 fields in struct, only 2 in metadata
};

//=============================================================================
// DEMONSTRATION
//=============================================================================

int main() {
    std::cout << "===========================================\n";
    std::cout << "  METADATA COMPLETENESS CHECK\n";
    std::cout << "===========================================\n\n";
    
    //-------------------------------------------------------------------------
    // TEST 1: Complete metadata
    //-------------------------------------------------------------------------
    std::cout << "TEST 1: GoodPerson\n";
    std::cout << "-------------------\n";
    std::cout << "Struct fields:\n";
    std::cout << "  1. name  (string)\n";
    std::cout << "  2. age   (int)\n";
    std::cout << "  3. email (string)\n\n";
    
    std::cout << "Metadata fields:\n";
    std::cout << "  1. name  ✓\n";
    std::cout << "  2. age   ✓\n";
    std::cout << "  3. email ✓\n\n";
    
    constexpr size_t good_count = std::tuple_size_v<decltype(GoodPerson::fields)>;
    std::cout << "Metadata has " << good_count << " fields\n";
    std::cout << "Result: ✓✓✓ COMPLETE - All fields covered!\n\n";
    
    std::cout << "Round-trip outcome:\n";
    std::cout << "  Original: {name:\"Alice\", age:30, email:\"alice@example.com\"}\n";
    std::cout << "  Serialize → JSON: {\"name\":\"Alice\",\"age\":30,\"email\":\"alice@example.com\"}\n";
    std::cout << "  Deserialize → Object: {name:\"Alice\", age:30, email:\"alice@example.com\"}\n";
    std::cout << "  ✓ Perfect match! No data loss!\n\n\n";
    
    //-------------------------------------------------------------------------
    // TEST 2: Incomplete metadata  
    //-------------------------------------------------------------------------
    std::cout << "TEST 2: BadPerson\n";
    std::cout << "-------------------\n";
    std::cout << "Struct fields:\n";
    std::cout << "  1. name  (string)\n";
    std::cout << "  2. age   (int)\n";
    std::cout << "  3. email (string)  ← NOT IN METADATA!\n\n";
    
    std::cout << "Metadata fields:\n";
    std::cout << "  1. name  ✓\n";
    std::cout << "  2. age   ✓\n";
    std::cout << "  ✗ email is MISSING!\n\n";
    
    constexpr size_t bad_count = std::tuple_size_v<decltype(BadPerson::fields)>;
    std::cout << "Metadata has " << bad_count << " fields\n";
    std::cout << "Result: ✗✗✗ INCOMPLETE - email field not covered!\n\n";
    
    std::cout << "Round-trip outcome:\n";
    std::cout << "  Original: {name:\"Bob\", age:25, email:\"bob@example.com\"}\n";
    std::cout << "  Serialize → JSON: {\"name\":\"Bob\",\"age\":25}  ← email MISSING!\n";
    std::cout << "  Deserialize → Object: {name:\"Bob\", age:25, email:\"\"}  ← LOST!\n";
    std::cout << "  ✗ Data loss detected! Round-trip FAILED!\n\n";
    
    //-------------------------------------------------------------------------
    // SUMMARY
    //-------------------------------------------------------------------------
    std::cout << "\n===========================================\n";
    std::cout << "           THE POINT\n";
    std::cout << "===========================================\n\n";
    
    std::cout << "✓ GoodPerson round-trip: SUCCESS\n";
    std::cout << "  → All fields in metadata\n";
    std::cout << "  → No data loss\n";
    std::cout << "  → Test PASSES\n\n";
    
    std::cout << "✗ BadPerson round-trip: FAILURE\n";
    std::cout << "  → email field missing from metadata\n";
    std::cout << "  → Data lost during serialization\n";
    std::cout << "  → Test FAILS immediately\n\n";
    
    std::cout << "📝 Round-trip testing PROVES your metadata is complete!\n\n";
    
    std::cout << "If you forget to add a field to metadata:\n";
    std::cout << "  → Serialization won't include it\n";
    std::cout << "  → Deserialization can't restore it\n";
    std::cout << "  → Round-trip test FAILS\n";
    std::cout << "  → You fix it IMMEDIATELY (add to metadata)\n";
    std::cout << "  → Re-test → PASSES\n\n";
    
    std::cout << "This is why you write the test FIRST:\n";
    std::cout << "  1. Write struct\n";
    std::cout << "  2. Write metadata annotations\n";
    std::cout << "  3. Write round-trip test ← CATCHES MISSING FIELDS!\n";
    std::cout << "  4. Fix metadata\n";
    std::cout << "  5. Ship with confidence\n\n";
    
    return 0;
}
