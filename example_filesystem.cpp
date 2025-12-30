#include <filesystem>
#include <iostream>

#include "meta.h"

using namespace meta;
struct Config {
  std::string appName;
  std::filesystem::path logPath;
  std::filesystem::path dataDir;
  int port;

  static constexpr auto FieldsMeta = std::make_tuple(
      field<&Config::appName>("appName"),
      field<&Config::logPath>("logPath"),
      field<&Config::dataDir>("dataDir"), field<&Config::port>("port"));
};

int main() {
  // Test serialization
  Config cfg{"MyApp", std::filesystem::path("/var/log/myapp.log"),
             std::filesystem::path("/opt/myapp/data"), 8080};

  std::cout << "=== YAML Output ===\n";
  std::cout << meta::toYaml(cfg) << "\n\n";

  std::cout << "=== JSON Output ===\n";
  std::cout << meta::toJson(cfg) << "\n\n";

  // Test deserialization
  std::string yaml = R"(
appName: TestApp
logPath: /tmp/test.log
dataDir: /home/user/data
port: 9000
)";

  auto [result, validation] = meta::reifyFromYaml<Config>(yaml);

  if (validation.valid) {
    std::cout << "=== Deserialized Successfully ===\n";
    std::cout << "App Name: " << result->appName << "\n";
    std::cout << "Log Path: " << result->logPath << "\n";
    std::cout << "Data Dir: " << result->dataDir << "\n";
    std::cout << "Port: " << result->port << "\n";

    // Verify paths work
    std::cout << "\nPath operations work:\n";
    std::cout << "Log parent: " << result->logPath.parent_path() << "\n";
    std::cout << "Log filename: " << result->logPath.filename() << "\n";
    std::cout << "Data dir string: " << result->dataDir.string() << "\n";
  } else {
    std::cout << "=== Deserialization Failed ===\n";
    for (const auto &[field, error] : validation.errors) {
      std::cout << "Error in " << field << ": " << error << "\n";
    }
  }

  return 0;
}
