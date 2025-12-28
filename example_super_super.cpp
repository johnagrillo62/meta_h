/*
 * mega_topology_full.cpp - Single-file mega complex topology serialization with
 * main
 */

#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "meta.h"

using namespace meta;
// ============================================================
// Basic structures
// ============================================================

struct PortConfig {
  int port;
  std::string protocol;
  std::string status;

  static constexpr auto fields = std::make_tuple(
      field<&PortConfig::port>("port", Description{"Port number"}),
      field<&PortConfig::protocol>("protocol", Description{"TCP/UDP"}),
      field<&PortConfig::status>("status", Description{"Port status"}));
};

struct Server {
  std::string hostname;
  std::vector<PortConfig> ports;
  std::map<std::string, std::string> metadata;

  static constexpr auto fields = std::make_tuple(
      field<&Server::hostname>("hostname", Description{"Server name"}),
      field<&Server::ports>("ports", Description{"Port configs"}),
      field<&Server::metadata>("metadata",
                                   Description{"Key-value metadata"}));
};

struct DataCenter {
  std::string name;
  std::vector<Server> servers;
  std::map<std::string, std::tuple<int, int, std::string>> networkRanges;

  static constexpr auto fields = std::make_tuple(
      field<&DataCenter::name>("name", Description{"DataCenter name"}),
      field<&DataCenter::servers>("servers", Description{"Servers"}),
      field<&DataCenter::networkRanges>("networkRanges",
                                            Description{"IP ranges"}));
};

struct MegaTopology {
  std::map<std::string, DataCenter> datacenters;
  std::vector<std::pair<std::string, std::string>> serviceEndpoints;
  std::string description;

  static constexpr auto fields = std::make_tuple(
      field<&MegaTopology::datacenters>("datacenters",
                                            Description{"All DCs"}),
      field<&MegaTopology::serviceEndpoints>("serviceEndpoints",
                                                 Description{"Services"}),
      field<&MegaTopology::description>("description",
                                            Description{"Description"}));
};

// ============================================================
// Generate huge topology
// ============================================================

MegaTopology generateHugeTopology(size_t regions = 50,
                                  size_t serversPerRegion = 20,
                                  size_t portsPerServer = 5) {
  MegaTopology topo;
  for (size_t r = 0; r < regions; ++r) {
    DataCenter dc;
    dc.name = "Region-" + std::to_string(r);
    for (size_t s = 0; s < serversPerRegion; ++s) {
      Server srv;
      srv.hostname = "server-" + std::to_string(r) + "-" + std::to_string(s);
      for (size_t p = 0; p < portsPerServer; ++p) {
        srv.ports.push_back({80 + int(p), "TCP", "open"});
      }
      srv.metadata = {{"os", "linux"}};
      dc.servers.push_back(srv);
    }
    dc.networkRanges["10." + std::to_string(r) + ".0.0"] = {1, 255, "active"};
    topo.datacenters[dc.name] = dc;
  }
  topo.description = "Massive topology test";
  topo.serviceEndpoints = {{"api-gateway", "server-0-0"}, {"db", "server-1-0"}};
  return topo;
}

// ============================================================
// Main
// ============================================================

int main() {
  std::cout << "Generating mega topology...\n";
  auto topo = generateHugeTopology();

  std::cout << "Serializing to JSON...\n";
  auto start = std::chrono::high_resolution_clock::now();
  auto json = meta::toJson(topo);
  auto end = std::chrono::high_resolution_clock::now();

  std::cout << "MegaTopology JSON size: " << json.size() << " bytes\n";
  std::cout << "Serialization took: "
            << std::chrono::duration<double, std::milli>(end - start).count()
            << " ms\n";

  std::cout << "JSON snippet: " << json.substr(0, 200) << "...\n";

  // Optional: serialize to YAML
  std::cout << "\nSerializing to YAML...\n";
  auto startY = std::chrono::high_resolution_clock::now();
  auto yaml = meta::toYaml(topo);
  auto endY = std::chrono::high_resolution_clock::now();
  std::cout << "YAML serialization took: "
            << std::chrono::duration<double, std::milli>(endY - startY).count()
            << " ms\n";

  std::cout << "YAML snippet:\n" << yaml.substr(0, 500) << "...\n";
}
