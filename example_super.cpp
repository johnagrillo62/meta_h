#include <iostream>
#include <map>
#include <optional>
#include <tuple>
#include <vector>

#include "meta.h"

using namespace meta;
// ============================================================================
// ENUM: Status
// ============================================================================

enum class Status { Offline, Online, Maintenance, Degraded };

// Register enum with meta framework
inline constexpr std::array StatusMapping = {
    std::pair{Status::Offline, "Offline"},
    std::pair{Status::Online, "Online"},
    std::pair{Status::Maintenance, "Maintenance"},
    std::pair{Status::Degraded, "Degraded"}
};

namespace meta {
    template <>
    struct EnumMapping<Status> {
        using Type = EnumTraitsAuto<Status, StatusMapping>;
    };
}

// ============================================================================
// STRUCT: PortConfig
// ============================================================================

struct PortConfig {
  int port;
  std::optional<std::string> protocol; // e.g., "TCP", "UDP"
  Status status;

  static constexpr auto FieldsMeta = std::make_tuple(
      field<&PortConfig::port>("port", Description{"Port number"}),
      field<&PortConfig::protocol>("protocol",
                                       Description{"Optional protocol"}),
      field<&PortConfig::status>("status", Description{"Port status"}));
};

// ============================================================================
// STRUCT: Server
// ============================================================================

struct Server {
  std::string hostname;
  std::vector<PortConfig> ports;
  std::map<std::string, std::string> metadata; // arbitrary key-value pairs

  static constexpr auto FieldsMeta = std::make_tuple(
      field<&Server::hostname>("hostname", Description{"Server name"}),
      field<&Server::ports>("ports", Description{"Port configurations"}),
      field<&Server::metadata>("metadata",
                                   Description{"Key-value metadata"}));
};

// ============================================================================
// STRUCT: DataCenter
// ============================================================================

struct DataCenter {
  std::string name;
  std::vector<Server> servers;
  std::map<std::string, std::tuple<int, int, Status>>
      networkRanges; // subnet: (start, end, status)

  static constexpr auto FieldsMeta = std::make_tuple(
      field<&DataCenter::name>("name", Description{"Datacenter name"}),
      field<&DataCenter::servers>("servers",
                                      Description{"Servers in datacenter"}),
      field<&DataCenter::networkRanges>(
          "networkRanges", Description{"Network ranges with status"}));
};

// ============================================================================
// STRUCT: GlobalTopology
// ============================================================================

struct GlobalTopology {
  std::map<std::string, DataCenter> datacenters; // region -> datacenter
  std::vector<std::tuple<std::string, std::string, Status>>
      serviceEndpoints; // service -> host -> status
  std::optional<std::string> description;

  static constexpr auto FieldsMeta = std::make_tuple(
      field<&GlobalTopology::datacenters>(
          "datacenters", Description{"Map of region->datacenter"}),
      field<&GlobalTopology::serviceEndpoints>(
          "serviceEndpoints", Description{"List of service endpoints"}),
      field<&GlobalTopology::description>(
          "description", Description{"Optional description"}));
};

// ============================================================================
// TEST: Mega Complex Serialization
// ============================================================================

void testMegaComplex() {
  GlobalTopology topo{
      {{"north-america",
        {"NA-1",
         {{"server1",
           {{80, "TCP", Status::Online}, {443, "TCP", Status::Maintenance}},
           {{"os", "linux"}}},
          {"server2",
           {{22, "TCP", Status::Online}, {8080, "TCP", Status::Degraded}},
           {}}},
         {{"10.0.0.0", std::make_tuple(1, 255, Status::Online)},
          {"10.0.1.0", std::make_tuple(1, 255, Status::Degraded)}}}},
       {"europe",
        {"EU-1",
         {
             {"eu-server1",
              {{80, "TCP", Status::Online}},
              {{"region", "eu-west"}}},
         },
         {{"192.168.0.0", std::make_tuple(1, 255, Status::Online)}}}}},
      {{"api-gateway", "server1", Status::Online},
       {"auth-service", "server2", Status::Degraded},
       {"db-primary", "eu-server1", Status::Online}},
      "Global multi-region topology"};

  std::cout << "Mega Complex Topology YAML:\n";
  std::cout << meta::toYaml(topo) << "\n\n";

  std::cout << "Mega Complex Topology JSON:\n";
  std::cout << meta::toJson(topo) << "\n\n";

  // Example: Deserialization can be done the same way
}

int main() {
  testMegaComplex();
  return 0;
}

