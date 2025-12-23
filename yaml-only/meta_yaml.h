/*
 * meta_yaml.h - Minimal C++20 YAML Serialization
 * 
 * Simple, zero-overhead YAML serialization for C++20
 * No builder pattern, no walkers - just direct YAML serialization.
 */

#pragma once

#include <iostream>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace meta {

// ============================================================================
// ENUM SUPPORT
// ============================================================================

template <typename T> struct EnumMapping;

template <typename EnumT, auto &MappingArray> struct EnumTraitsAuto {
  inline static constexpr auto &mapping = MappingArray;

  inline static const std::unordered_map<EnumT, std::string> enumToString =
      []() {
        std::unordered_map<EnumT, std::string> m;
        for (auto [e, s] : mapping)
          m[e] = s;
        return m;
      }();

  inline static const std::unordered_map<std::string, EnumT> stringToEnum =
      []() {
        std::unordered_map<std::string, EnumT> m;
        for (auto [e, s] : mapping)
          m[s] = e;
        return m;
      }();

  inline static std::string toString(EnumT e) {
    auto it = enumToString.find(e);
    return it != enumToString.end() ? it->second : "";
  }

  inline static std::optional<EnumT> fromString(const std::string &s) {
    auto it = stringToEnum.find(s);
    return it != stringToEnum.end() ? std::optional(it->second) : std::nullopt;
  }

  inline static std::string getValidValues() {
    std::string result;
    bool first = true;
    for (const auto &[str, _] : stringToEnum) {
      if (!first) result += ", ";
      result += "'" + str + "'";
      first = false;
    }
    return result;
  }

  template <typename Func> inline static void forEach(Func f) {
    for (auto [e, _] : mapping)
      f(e);
  }
};

template <typename EnumT>
concept RegisteredEnum = requires { typename EnumMapping<EnumT>::Type; };

template <typename EnumT>
  requires RegisteredEnum<EnumT>
inline std::ostream &operator<<(std::ostream &os, EnumT e);

// ============================================================================
// FIELD METADATA
// ============================================================================

enum class Requirement { Required, Optional };
constexpr auto OptionalField = Requirement::Optional;

template <typename MemberPtr> struct member_pointer_traits;

template <typename T, typename Class> struct member_pointer_traits<T Class::*> {
  using type = T;
};

template <auto MemberPtr> struct Field {
  using member_type = typename member_pointer_traits<decltype(MemberPtr)>::type;
  std::string_view fieldName;
  Requirement requirement;
  static constexpr auto memberPtr = MemberPtr;

  inline constexpr Field(std::string_view name, Requirement req = Requirement::Required)
      : fieldName(name), requirement(req) {}
};

// ============================================================================
// VALIDATION RESULT
// ============================================================================

struct ValidationResult {
  bool valid = true;
  std::vector<std::pair<std::string, std::string>> errors;
  
  inline void addError(std::string_view field, std::string_view msg) {
    valid = false;
    errors.push_back({std::string(field), std::string(msg)});
  }
};

// ============================================================================
// YAML NODE WRAPPER
// ============================================================================

struct Node {
  YAML::Node node;

  inline Node(YAML::Node n) : node(n) {}

  inline std::optional<int> asInt() const {
    try { return node.as<int>(); } catch (...) { return {}; }
  }

  inline std::optional<double> asDouble() const {
    try { return node.as<double>(); } catch (...) { return {}; }
  }

  inline std::optional<bool> asBool() const {
    try { return node.as<bool>(); } catch (...) { return {}; }
  }

  inline std::optional<std::string> asString() const {
    try { return node.as<std::string>(); } catch (...) { return {}; }
  }

  inline bool isSequence() const { return node.IsSequence(); }
  inline bool isMap() const { return node.IsMap(); }
  inline bool isNull() const { return node.IsNull(); }
  inline size_t size() const { return node.size(); }

  inline Node at(size_t i) const { return Node(node[i]); }
  inline Node at(const std::string &k) const { return Node(node[k]); }
  inline std::vector<std::string> keys() const {
    std::vector<std::string> k;
    for (auto &item : node)
      if (item.first.IsScalar())
        k.push_back(item.first.as<std::string>());
    return k;
  }
};

// ============================================================================
// DESERIALIZATION
// ============================================================================

template <typename T> inline ValidationResult from(T &obj, const Node &node);

// Primitives
template <>
inline ValidationResult from(int &obj, const Node &node) {
  auto v = node.asInt();
  if (!v) {
    ValidationResult r;
    r.addError("", "Invalid integer");
    return r;
  }
  obj = *v;
  return ValidationResult();
}

template <>
inline ValidationResult from(double &obj, const Node &node) {
  auto v = node.asDouble();
  if (!v) v = node.asInt();
  if (!v) {
    ValidationResult r;
    r.addError("", "Invalid double");
    return r;
  }
  obj = *v;
  return ValidationResult();
}

template <>
inline ValidationResult from(std::string &obj, const Node &node) {
  auto v = node.asString();
  if (!v) {
    ValidationResult r;
    r.addError("", "Invalid string");
    return r;
  }
  obj = *v;
  return ValidationResult();
}

template <>
inline ValidationResult from(bool &obj, const Node &node) {
  auto v = node.asBool();
  if (!v) {
    ValidationResult r;
    r.addError("", "Invalid boolean");
    return r;
  }
  obj = *v;
  return ValidationResult();
}

template <>
inline ValidationResult from(std::filesystem::path &obj, const Node &node) {
  auto str = node.asString();
  if (!str) {
    ValidationResult r;
    r.addError("", "Invalid path");
    return r;
  }
  obj = std::filesystem::path(*str);
  return ValidationResult();
}

// Enums
template <RegisteredEnum EnumT>
inline ValidationResult from(EnumT &obj, const Node &node) {
  auto s = node.asString();
  if (!s) {
    ValidationResult r;
    std::string validValues = EnumMapping<EnumT>::Type::getValidValues();
    r.addError("", "Invalid enum. Valid values are: " + validValues);
    return r;
  }
  auto e = EnumMapping<EnumT>::Type::fromString(*s);
  if (!e) {
    ValidationResult r;
    std::string validValues = EnumMapping<EnumT>::Type::getValidValues();
    r.addError("", "Unknown enum value '" + *s + "'. Valid values are: " + validValues);
    return r;
  }
  obj = *e;
  return ValidationResult();
}

// Vector
template <typename T>
inline ValidationResult from(std::vector<T> &obj, const Node &node) {
  ValidationResult result;
  if (!node.isSequence()) {
    result.addError("", "Expected sequence");
    return result;
  }
  obj.clear();
  for (size_t i = 0; i < node.size(); ++i) {
    T elem{};
    auto elemResult = from(elem, node.at(i));
    if (!elemResult.valid) {
      for (auto &[f, e] : elemResult.errors) {
        result.addError("[" + std::to_string(i) + "]" + (f.empty() ? "" : "." + f), e);
      }
    } else {
      obj.push_back(elem);
    }
  }
  return result;
}

// Map
template <typename K, typename V>
inline ValidationResult from(std::map<K, V> &obj, const Node &node) {
  ValidationResult result;
  if (!node.isMap()) {
    result.addError("", "Expected map");
    return result;
  }
  obj.clear();
  for (auto &k : node.keys()) {
    V v{};
    auto vResult = from(v, node.at(k));
    if (!vResult.valid) {
      for (auto &[f, e] : vResult.errors) {
        result.addError(k + (f.empty() ? "" : "." + f), e);
      }
    } else {
      obj[k] = v;
    }
  }
  return result;
}

// Optional
template <typename T>
inline ValidationResult from(std::optional<T> &obj, const Node &node) {
  if (node.isNull()) {
    obj = std::nullopt;
    return ValidationResult();
  }
  T v{};
  auto result = from(v, node);
  if (result.valid)
    obj = v;
  return result;
}

// Pair
template <typename K, typename V>
inline ValidationResult from(std::pair<K, V> &obj, const Node &node) {
  ValidationResult result;
  if (!node.isSequence() || node.size() != 2) {
    result.addError("", "Expected sequence of 2 elements");
    return result;
  }
  auto kResult = from(obj.first, node.at(0));
  auto vResult = from(obj.second, node.at(1));
  if (!kResult.valid) result = kResult;
  if (!vResult.valid) result = vResult;
  return result;
}

// Tuple
template <typename... Args>
inline ValidationResult from(std::tuple<Args...> &obj, const Node &node) {
  ValidationResult result;
  if (!node.isSequence() || node.size() != sizeof...(Args)) {
    result.addError("", "Expected sequence");
    return result;
  }
  std::apply(
      [&](auto &&...elems) {
        size_t i = 0;
        (..., [&](auto &elem) {
          auto elemResult = from(elem, node.at(i++));
          if (!elemResult.valid) {
            for (auto &[f, e] : elemResult.errors) {
              result.addError("[" + std::to_string(i - 1) + "]" + (f.empty() ? "" : "." + f), e);
            }
          }
        }(elems));
      },
      obj);
  return result;
}

// Struct
template <typename T>
inline ValidationResult from(T &obj, const Node &node) {
  if constexpr (requires { T::fields; }) {
    ValidationResult result;
    std::apply(
        [&](auto &&...fields) {
          (..., [&](auto &field) {
            auto fieldNode = node.at(std::string(field.fieldName));
            if (fieldNode.isNull()) {
              if (field.requirement == Requirement::Required) {
                result.addError(std::string(field.fieldName), "Missing required field");
              }
              return;
            }
            auto fieldResult = from(obj.*(field.memberPtr), fieldNode);
            if (!fieldResult.valid) {
              for (auto &[f, e] : fieldResult.errors) {
                result.addError(std::string(field.fieldName) + (f.empty() ? "" : "." + f), e);
              }
            }
          }(fields));
        },
        T::fields);
    return result;
  } else {
    static_assert(requires { T::fields; }, "Type must have static constexpr fields tuple");
    return ValidationResult();
  }
}

// ============================================================================
// SERIALIZATION
// ============================================================================

template <typename T> inline YAML::Node to(const T &obj);

// Primitives
template <>
inline YAML::Node to(const int &obj) {
  return YAML::Node(obj);
}

template <>
inline YAML::Node to(const double &obj) {
  return YAML::Node(obj);
}

template <>
inline YAML::Node to(const bool &obj) {
  return YAML::Node(obj);
}

template <>
inline YAML::Node to(const std::string &obj) {
  return YAML::Node(obj);
}

template <>
inline YAML::Node to(const std::filesystem::path &obj) {
  return YAML::Node(obj.string());
}

// Enums
template <RegisteredEnum EnumT>
inline YAML::Node to(const EnumT &obj) {
  return YAML::Node(EnumMapping<EnumT>::Type::toString(obj));
}

// Vector
template <typename T>
inline YAML::Node to(const std::vector<T> &obj) {
  YAML::Node node;
  for (size_t i = 0; i < obj.size(); ++i) {
    node[i] = to(obj[i]);
  }
  return node;
}

// Map
template <typename K, typename V>
inline YAML::Node to(const std::map<K, V> &obj) {
  YAML::Node node;
  for (const auto &[k, v] : obj) {
    node[k] = to(v);
  }
  return node;
}

// Optional
template <typename T>
inline YAML::Node to(const std::optional<T> &obj) {
  if (obj) return to(*obj);
  return YAML::Node();
}

// Pair
template <typename K, typename V>
inline YAML::Node to(const std::pair<K, V> &obj) {
  YAML::Node node;
  node[0] = to(obj.first);
  node[1] = to(obj.second);
  return node;
}

// Tuple
template <typename... Args>
inline YAML::Node to(const std::tuple<Args...> &obj) {
  YAML::Node node;
  size_t i = 0;
  std::apply(
      [&](const auto &...args) {
        (..., [&](const auto &arg) {
          node[i++] = to(arg);
        }(args));
      },
      obj);
  return node;
}

// Struct
template <typename T>
inline YAML::Node to(const T &obj) {
  if constexpr (requires { T::fields; }) {
    YAML::Node node;
    std::apply(
        [&](auto &&...fields) {
          (..., [&](auto &field) {
            node[std::string(field.fieldName)] = to(obj.*(field.memberPtr));
          }(fields));
        },
        T::fields);
    return node;
  } else {
    static_assert(requires { T::fields; }, "Type must have static constexpr fields tuple");
    return YAML::Node();
  }
}

// ============================================================================
// PUBLIC API
// ============================================================================

template <typename T>
inline std::pair<std::optional<T>, ValidationResult>
fromYaml(const std::string &yaml) {
  try {
    YAML::Node node = YAML::Load(yaml);
    T obj{};
    auto result = from(obj, Node(node));
    return result.valid ? std::make_pair(std::optional(obj), result)
                        : std::make_pair(std::nullopt, result);
  } catch (const std::exception &e) {
    ValidationResult result;
    result.addError("yaml", std::string(e.what()));
    return {std::nullopt, result};
  }
}

template <typename T>
inline std::string toYaml(const T &obj) {
  YAML::Emitter emitter;
  emitter << to(obj);
  return emitter.c_str();
}

// ============================================================================
// ENUM HELPERS
// ============================================================================

template <typename EnumT>
  requires RegisteredEnum<EnumT>
inline std::optional<EnumT> toEnum(const std::string &s) {
  return EnumMapping<EnumT>::Type::fromString(s);
}

template <typename EnumT>
  requires RegisteredEnum<EnumT>
inline std::vector<EnumT> enumValues() {
  std::vector<EnumT> values;
  EnumMapping<EnumT>::Type::forEach([&](EnumT e) { values.push_back(e); });
  return values;
}

template <typename EnumT>
  requires RegisteredEnum<EnumT>
inline std::ostream &operator<<(std::ostream &os, EnumT e) {
  os << EnumMapping<EnumT>::Type::toString(e);
  return os;
}

} // namespace meta

// ============================================================================
// GLOBAL ENUM OPERATOR
// ============================================================================

template <typename EnumT>
  requires meta::RegisteredEnum<EnumT>
inline std::ostream &operator<<(std::ostream &os, EnumT e) {
  os << meta::EnumMapping<EnumT>::Type::toString(e);
  return os;
}

