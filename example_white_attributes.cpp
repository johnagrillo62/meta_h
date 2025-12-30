#include "meta.h"
#include <iostream>

using namespace meta;

// ============================================================================
// WHITELIST EXAMPLES WITH DIFFERENT TYPES
// ============================================================================

// Strings
constexpr std::array allowed_envs = {
    std::string_view("dev"), 
    std::string_view("staging"), 
    std::string_view("prod")
};

// Integers
constexpr std::array allowed_ports = {80, 443, 8080, 8443};

// Floats
constexpr std::array allowed_versions = {1.0f, 1.5f, 2.0f, 2.5f};

// ============================================================================

struct Config {
    std::string environment;
    int port;
    float version;
    
    static constexpr auto FieldsMeta = std::make_tuple(
        field<&Config::environment>("environment", 
            Whitelist<allowed_envs>{}),  // String whitelist
        
        field<&Config::port>("port", 
            Whitelist<allowed_ports>{}),  // Int whitelist
        
        field<&Config::version>("version", 
            Whitelist<allowed_versions>{})  // Float whitelist
    );
};

int main() {
    // Valid config
    std::string valid_yaml = R"(
environment: prod
port: 443
version: 2.0
)";
    
    auto [cfg1, result1] = meta::reifyFromYaml<Config>(valid_yaml);
    if (result1.valid) {
        std::cout << "Valid config parsed!\n";
        std::cout << meta::toYaml(*cfg1) << "\n";
    }
    
    // Invalid environment (string)
    std::string bad_env = R"(
environment: testing
port: 443
version: 2.0
)";
    
    auto [cfg2, result2] = meta::reifyFromYaml<Config>(bad_env);
    if (!result2.valid) {
        std::cout << "\nInvalid environment:\n";
        for (auto& [field, err] : result2.errors) {
            std::cout << "  " << field << ": " << err << "\n";
        }
        // Should show: Value not in whitelist: {'dev', 'staging', 'prod'}
    }
    
    // Invalid port (int)
    std::string bad_port = R"(
environment: prod
port: 9999
version: 2.0
)";
    
    auto [cfg3, result3] = meta::reifyFromYaml<Config>(bad_port);
    if (!result3.valid) {
        std::cout << "\nInvalid port:\n";
        for (auto& [field, err] : result3.errors) {
            std::cout << "  " << field << ": " << err << "\n";
        }
        // Should show: Value not in whitelist: {80, 443, 8080, 8443}
    }
    
    // Invalid version (float)
    std::string bad_version = R"(
environment: prod
port: 443
version: 3.0
)";
    
    auto [cfg4, result4] = meta::reifyFromYaml<Config>(bad_version);
    if (!result4.valid) {
        std::cout << "\nInvalid version:\n";
        for (auto& [field, err] : result4.errors) {
            std::cout << "  " << field << ": " << err << "\n";
        }
        // Should show: Value not in whitelist: {1.0, 1.5, 2.0, 2.5}
    }
    
    return 0;
}

