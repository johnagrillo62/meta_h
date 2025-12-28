

// example_sfinae_working.cpp - Boost.PFR Technique with C++20
//
// Working approach:
// 1. SFINAE field counting via aggregate initialization attempt
// 2. Structured bindings to decompose (outside requires clauses)
// 3. Automatic serialization - ZERO manual registration!

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

namespace meta {

// ============================================================================
// FIELD COUNTING - Try initializing with braced-init-lists
// ============================================================================

namespace detail {
// Helper to detect field count using brace initialization
struct any_type {
  template <typename T> constexpr operator T();
};

template <typename T, typename... Args>
concept can_be_initialized = requires(T t) { T{std::declval<Args>()...}; };

// Count fields by trying increasingly longer initializer lists
template <typename T> consteval size_t count_fields() {
  if constexpr (can_be_initialized<T, any_type, any_type, any_type, any_type,
                                   any_type, any_type, any_type, any_type>)
    return 8;
  else if constexpr (can_be_initialized<T, any_type, any_type, any_type,
                                        any_type, any_type, any_type, any_type>)
    return 7;
  else if constexpr (can_be_initialized<T, any_type, any_type, any_type,
                                        any_type, any_type, any_type>)
    return 6;
  else if constexpr (can_be_initialized<T, any_type, any_type, any_type,
                                        any_type, any_type>)
    return 5;
  else if constexpr (can_be_initialized<T, any_type, any_type, any_type,
                                        any_type>)
    return 4;
  else if constexpr (can_be_initialized<T, any_type, any_type, any_type>)
    return 3;
  else if constexpr (can_be_initialized<T, any_type, any_type>)
    return 2;
  else if constexpr (can_be_initialized<T, any_type>)
    return 1;
  else
    return 0;
}
} // namespace detail

template <typename T> constexpr size_t field_count = detail::count_fields<T>();

// ============================================================================
// AUTO DECOMPOSE using Structured Bindings
// ============================================================================

template <typename T> auto as_tuple(T &obj) {
  constexpr size_t N = field_count<T>;

  if constexpr (N == 1) {
    auto &[a] = obj;
    return std::tie(a);
  } else if constexpr (N == 2) {
    auto &[a, b] = obj;
    return std::tie(a, b);
  } else if constexpr (N == 3) {
    auto &[a, b, c] = obj;
    return std::tie(a, b, c);
  } else if constexpr (N == 4) {
    auto &[a, b, c, d] = obj;
    return std::tie(a, b, c, d);
  } else if constexpr (N == 5) {
    auto &[a, b, c, d, e] = obj;
    return std::tie(a, b, c, d, e);
  } else if constexpr (N == 6) {
    auto &[a, b, c, d, e, f] = obj;
    return std::tie(a, b, c, d, e, f);
  } else if constexpr (N == 7) {
    auto &[a, b, c, d, e, f, g] = obj;
    return std::tie(a, b, c, d, e, f, g);
  } else if constexpr (N == 8) {
    auto &[a, b, c, d, e, f, g, h] = obj;
    return std::tie(a, b, c, d, e, f, g, h);
  } else {
    static_assert(false, "Type must have 1-8 fields");
  }
}

// ============================================================================
// SERIALIZE - ZERO REGISTRATION NEEDED
// ============================================================================

template <typename T> std::string serialize(const T &value, int indent = 0);

inline std::string indent_str(int level) { return std::string(level * 2, ' '); }

// Primitives - int
template <> inline std::string serialize<int>(const int &value, int) {
  return std::to_string(value);
}

// Primitives - bool
template <> inline std::string serialize<bool>(const bool &value, int) {
  return value ? "true" : "false";
}

// Primitives - double
template <> inline std::string serialize<double>(const double &value, int) {
  return std::to_string(value);
}

// Primitives - string
template <>
inline std::string serialize<std::string>(const std::string &value, int) {
  return "\"" + value + "\"";
}

// Enums
template <typename E>
  requires std::is_enum_v<E>
std::string serialize(const E &value, int) {
  return std::to_string(static_cast<int>(value));
}

// Vectors
template <typename T>
std::string serialize(const std::vector<T> &vec, int indent) {
  if (vec.empty())
    return "[]";

  std::ostringstream ss;
  ss << "[\n";
  for (size_t i = 0; i < vec.size(); i++) {
    ss << indent_str(indent + 1);
    ss << serialize(vec[i], indent + 1);
    if (i + 1 < vec.size())
      ss << ",";
    ss << "\n";
  }
  ss << indent_str(indent) << "]";
  return ss.str();
}

// Structs - AUTO using structured bindings (NO REGISTRATION!)
template <typename T>
  requires(!std::is_enum_v<T> && std::is_aggregate_v<T> &&
           !std::is_same_v<T, std::string>)
std::string serialize(const T &obj, int indent) {
  std::ostringstream ss;
  ss << "{\n";

  auto tuple = as_tuple(const_cast<T &>(obj));
  constexpr size_t N = field_count<T>;

  std::apply(
      [&](auto &&...fields) {
        size_t idx = 0;
        (..., [&](auto &&field) {
          ss << indent_str(indent + 1);
          ss << "field" << idx << ": ";
          ss << serialize(field, indent + 1);
          if (idx + 1 < N)
            ss << ",";
          ss << "\n";
          idx++;
        }(fields));
      },
      tuple);

  ss << indent_str(indent) << "}";
  return ss.str();
}

// ============================================================================
// PUBLIC API
// ============================================================================

template <typename T> std::string to_string(const T &obj) {
  return serialize(obj, 0);
}

} // namespace meta

// ============================================================================
// TEST CODE
// ============================================================================

enum class Status { Pending = 0, Active = 1, Done = 2 };

// Just define the struct - NO manual registration!
struct Task {
  std::string name;
  Status status;
  int priority;
};

struct Project {
  std::string title;
  std::vector<Task> tasks;
};

int main() {
  std::cout << "=== Working C++20 Reflective Serializer ===\n\n";

  // Test 1: Field counting
  std::cout << "Test 1: SFINAE field counting\n";
  std::cout << "  Task: " << meta::field_count<Task> << " fields\n";
  std::cout << "  Project: " << meta::field_count<Project> << " fields\n\n";

  // Test 2: Simple struct
  std::cout << "Test 2: Simple struct serialization\n";
  Task t{"Fix bug", Status::Active, 1};
  std::cout << meta::to_string(t) << "\n\n";

  // Test 3: Nested structures
  std::cout << "Test 3: Nested structures\n";
  Project p{
      "MyProject",
      {{"Fix bug", Status::Active, 1}, {"Add feature", Status::Pending, 2}}};
  std::cout << meta::to_string(p) << "\n\n";

  std::cout << "✓ SFINAE: Count fields automatically\n";
  std::cout << "✓ Structured bindings: Decompose without registration\n";
  std::cout << "✓ C++20 concepts: Type-safe requirements\n";
  std::cout << "✓ Fold expressions: Process arbitrary field counts\n";

  return 0;
}
