#include <iostream>
#include <map>
#include <optional>
#include <tuple>
#include <vector>

#include "meta.h"

// ============================================================================
// ENUM: Status
// ============================================================================

enum class Status
{
    Offline,
    Online,
    Maintenance,
    Degraded
};

static constexpr auto StatusNames = std::make_tuple("Offline", "Online", "Maintenance", "Degraded");

// ============================================================================
// STRUCT: PortConfig
// ============================================================================

struct PortConfig
{
    int port;
    std::optional<std::string> protocol; // e.g., "TCP", "UDP"
    Status status;

    static constexpr auto fields =
        std::make_tuple(meta::Field<&PortConfig::port>{"port", "Port number"},
                        meta::Field<&PortConfig::protocol>{"protocol", "Optional protocol"},
                        meta::Field<&PortConfig::status>{"status", "Port status"});
};

// ============================================================================
// STRUCT: Server
// ============================================================================

struct Server
{
    std::string hostname;
    std::vector<PortConfig> ports;
    std::map<std::string, std::string> metadata; // arbitrary key-value pairs

    static constexpr auto fields =
        std::make_tuple(meta::Field<&Server::hostname>{"hostname", "Server name"},
                        meta::Field<&Server::ports>{"ports", "Port configurations"},
                        meta::Field<&Server::metadata>{"metadata", "Key-value metadata"});
};

// ============================================================================
// STRUCT: DataCenter
// ============================================================================

struct DataCenter
{
    std::string name;
    std::vector<Server> servers;
    std::map<std::string, std::tuple<int, int, Status>>
        networkRanges; // subnet: (start, end, status)

    static constexpr auto fields = std::make_tuple(
        meta::Field<&DataCenter::name>{"name", "Datacenter name"},
        meta::Field<&DataCenter::servers>{"servers", "Servers in datacenter"},
        meta::Field<&DataCenter::networkRanges>{"networkRanges", "Network ranges with status"});
};

// ============================================================================
// STRUCT: GlobalTopology
// ============================================================================

struct GlobalTopology
{
    std::map<std::string, DataCenter> datacenters; // region -> datacenter
    std::vector<std::tuple<std::string, std::string, Status>>
        serviceEndpoints; // service -> host -> status
    std::optional<std::string> description;

    static constexpr auto fields = std::make_tuple(
        meta::Field<&GlobalTopology::datacenters>{"datacenters", "Map of region->datacenter"},
        meta::Field<&GlobalTopology::serviceEndpoints>{"serviceEndpoints",
                                                       "List of service endpoints"},
        meta::Field<&GlobalTopology::description>{"description", "Optional description"});
};

// ============================================================================
// TEST: Mega Complex Serialization
// ============================================================================

void testMegaComplex()
{
    GlobalTopology topo{
        {{"north-america",
          {"NA-1",
           {{"server1",
             {{80, "TCP", Status::Online}, {443, "TCP", Status::Maintenance}},
             {{"os", "linux"}}},
            {"server2", {{22, "TCP", Status::Online}, {8080, "TCP", Status::Degraded}}, {}}},
           {{"10.0.0.0", std::make_tuple(1, 255, Status::Online)},
            {"10.0.1.0", std::make_tuple(1, 255, Status::Degraded)}}}},
         {"europe",
          {"EU-1",
           {
               {"eu-server1", {{80, "TCP", Status::Online}}, {{"region", "eu-west"}}},
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

int main()
{
    testMegaComplex();
    return 0;
}
