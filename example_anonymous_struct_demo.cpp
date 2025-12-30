
// anonymous_struct_demo_working.cpp - Manual decltype() annotations!
#include <iostream>
#include "meta.h"

// REAL-WORLD EXAMPLE: Hardware Register Definition
struct GPIO_Port {
    uint32_t address;
    
    // Mode register - anonymous struct
    struct {
      uint8_t input ;//: 1;
      uint8_t output;// : 1;
      uint8_t pullup;// : 1;
      uint8_t reserved;// : 5;
    } mode;
    
    // Control register - nested anonymous
    struct {
        uint8_t enable;
        struct {
	  uint8_t speed;// : 2;
	  uint8_t drive;// : 2;
	  uint8_t reserved;// : 4;
        } config;
    } control;
};

namespace meta
{
namespace GPIO_Port
{
inline const auto FieldsMeta = std::make_tuple(
    meta::field<&::GPIO_Port::address>("address"),
    meta::field<&::GPIO_Port::mode>("mode"),
    meta::field<&::GPIO_Port::control>("control"));

namespace mode
{
inline const auto FieldsMeta = std::make_tuple(
    meta::field<&decltype(::GPIO_Port::mode)::input>("input"),
    meta::field<&decltype(::GPIO_Port::mode)::output>("output"),
    meta::field<&decltype(::GPIO_Port::mode)::pullup>("pullup"),
    meta::field<&decltype(::GPIO_Port::mode)::reserved>("reserved"));

} // namespace mode
namespace control
{
inline const auto FieldsMeta = std::make_tuple(
    meta::field<&decltype(::GPIO_Port::control)::enable>("enable"),
    meta::field<&decltype(::GPIO_Port::control)::config>("config"));

namespace config
{
inline const auto FieldsMeta = std::make_tuple(
    meta::field<&decltype(decltype(::GPIO_Port::control)::config)::speed>("speed"),
    meta::field<&decltype(decltype(::GPIO_Port::control)::config)::drive>("drive"),
    meta::field<&decltype(decltype(::GPIO_Port::control)::config)::reserved>("reserved"));

} // namespace config
} // namespace control
} // namespace GPIO_Port
} // namespace meta

namespace meta
{
template <> struct MetaTuple<::GPIO_Port>
{
  static inline const auto& FieldsMeta = meta::GPIO_Port::FieldsMeta;
  static constexpr auto tableName = "GPIO_Port";
  static constexpr auto query = "select address, mode, control from GPIO_Port";
};
template <> struct MetaTuple<decltype(::GPIO_Port::mode)>
{
  static inline const auto& FieldsMeta = meta::GPIO_Port::mode::FieldsMeta;
};
template <> struct MetaTuple<decltype(::GPIO_Port::control)>
{
  static inline const auto& FieldsMeta = meta::GPIO_Port::control::FieldsMeta;
};
template <> struct MetaTuple<decltype(decltype(::GPIO_Port::control)::config)>
{
  static inline const auto& FieldsMeta = meta::GPIO_Port::control::config::FieldsMeta;
};
} // namespace meta


int main() {
    GPIO_Port port;
    port.address = 0x40020000;
    port.mode.input = 1;
    port.mode.output = 0;
    port.mode.pullup = 1;
    port.mode.reserved = 0;
    port.control.enable = 1;
    port.control.config.speed = 2;
    port.control.config.drive = 3;
    port.control.config.reserved = 0;
    
    std::cout << "=== Anonymous Struct Demo ===\n\n";
    
    std::cout << "YAML:\n";
    std::cout << meta::toYaml(port) << "\n";
    
    std::cout << "JSON:\n";
    std::cout << meta::toJson(port) << "\n";
    
    return 0;
}

