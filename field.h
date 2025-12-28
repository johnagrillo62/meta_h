/*
 * field.h - Field metadata for reflection
 * 
 * Provides Field<> template for storing compile-time metadata about struct members
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <optional>
#include <vector>
#include <sstream>

namespace meta
{

// ============================================================================
// FIELD ATTRIBUTES
// ============================================================================

enum class Requirement
{
    Required,
    Optional
};

struct Description
{
    std::string_view value;
};

struct Default
{
    std::string_view value;
};

struct Validator
{
    std::string_view rule;
};

// Column name overrides for different formats
struct CsvColumn
{
    std::string_view name;
};

struct SqlColumn
{
    std::string_view name;
};

struct JsonColumn
{
    std::string_view name;
};

struct CleanName
{
    std::string_view name;
};

// Properties flags
enum Prop : uint8_t
{
    None = 0,
    PrimaryKey = 1 << 0,
    Indexed = 1 << 1,
    Unique = 1 << 2,
    NotNull = 1 << 3,
    Serializable = 1 << 4,
    Hashable = 1 << 5
};

constexpr Prop operator|(Prop a, Prop b)
{
    return static_cast<Prop>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr Prop operator&(Prop a, Prop b)
{
    return static_cast<Prop>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline std::string propsToString(Prop props)
{
    if (props == Prop::None)
        return "None";
    
    std::vector<std::string> flags;
    
    if (props & Prop::PrimaryKey)
        flags.push_back("PrimaryKey");
    if (props & Prop::Indexed)
        flags.push_back("Indexed");
    if (props & Prop::Unique)
        flags.push_back("Unique");
    if (props & Prop::NotNull)
        flags.push_back("NotNull");
    if (props & Prop::Serializable)
        flags.push_back("Serializable");
    if (props & Prop::Hashable)
        flags.push_back("Hashable");
    
    std::string result;
    for (size_t i = 0; i < flags.size(); ++i) {
        if (i > 0) result += " | ";
        result += flags[i];
    }
    
    return result;
}

struct Props
{
    Prop value;
    constexpr Props(Prop p) : value(p) {}
};

// ============================================================================
// VALIDATION ATTRIBUTES
// ============================================================================

// Concept for numeric types
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <auto Min, auto Max>
struct BoundsCheck
{
    static constexpr auto min = Min;
    static constexpr auto max = Max;
    
    template <Numeric T>  // Now requires numeric type!
    static bool validate(const T& value, std::string& error)
    {
        if (value < Min || value > Max) {
            error = "Value " + std::to_string(value) + " out of bounds [" +
                    std::to_string(Min) + ", " + std::to_string(Max) + "]";
            return false;
        }
        return true;
    }
};

template <size_t Min, size_t Max>
struct StringLength
{
    static constexpr size_t min = Min;
    static constexpr size_t max = Max;
    
    // Explicitly requires std::string
    static bool validate(const std::string& value, std::string& error)
    {
        if (value.length() < Min || value.length() > Max) {
            error = "String length " + std::to_string(value.length()) +
                    " out of bounds [" + std::to_string(Min) + ", " + std::to_string(Max) + "]";
            return false;
        }
        return true;
    }
};

template <const auto& AllowedValues>
struct Whitelist
{
    template <typename T>
    static bool validate(const T& value, std::string& error)
    {
        for (const auto& allowed : AllowedValues) {
            if (value == allowed) return true;
        }
        
        // Simple: just use operator<< for everything
        std::ostringstream oss;
        oss << "Value not in whitelist: {";
        bool first = true;
        for (const auto& allowed : AllowedValues) {

        if (!first) oss << ", ";
            first = false;
            oss << allowed;
        }
        oss << "}";
        error = oss.str();
        return false;
    }
};


// ============================================================================
// TYPE NAME EXTRACTION
// ============================================================================

template <typename T>
constexpr std::string_view type_name()
{
#if defined(__GNUC__) || defined(__clang__)
    constexpr std::string_view func = __PRETTY_FUNCTION__;
    constexpr std::string_view prefix = "T = ";
    constexpr std::string_view suffix = "]";
    
    constexpr auto start = func.find(prefix) + prefix.size();
    constexpr auto end = func.rfind(suffix);
    
    return func.substr(start, end - start);
    
#elif defined(_MSC_VER)
    constexpr std::string_view func = __FUNCSIG__;
    constexpr std::string_view prefix = "type_name<";
    constexpr std::string_view suffix = ">(void)";
    
    constexpr auto start = func.find(prefix) + prefix.size();
    constexpr auto end = func.find(suffix);
    
    return func.substr(start, end - start);
    
#else
    return "unknown";
#endif
}

// ============================================================================
// MEMBER POINTER TRAITS
// ============================================================================

template <typename MemberPtr>
struct member_pointer_traits;

template <typename T, typename Class>
struct member_pointer_traits<T Class::*>
{
    using type = T;
    using class_type = Class;
    using member_type = T;
};

// ============================================================================
// TYPE DETECTION
// ============================================================================

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

// ============================================================================
// FIELD TEMPLATE
// ============================================================================

template <auto MemberPtr, typename... Attrs>
struct Field
{
    using MemberType = typename member_pointer_traits<decltype(MemberPtr)>::member_type;
    using ClassType = typename member_pointer_traits<decltype(MemberPtr)>::class_type;

    static constexpr auto memberPtr = MemberPtr;
    
    const char* fieldName;
    Requirement requirement;
    std::tuple<Attrs...> attributes;
    
    // Static constexpr check for attribute presence
    template<typename Attr>
    static constexpr bool has = (std::is_same_v<Attr, Attrs> || ...);
    
    // Constructor
    template <typename... Args>
    constexpr Field(const char* name, Args&&... args)
        : fieldName(name), 
          requirement(is_optional_v<MemberType> ? Requirement::Optional : Requirement::Required),
          attributes(std::make_tuple(std::forward<Args>(args)...))
    {}
    
    // Get type name
    static constexpr std::string_view getTypeName()
    {
        return type_name<MemberType>();
    }
    
    // Get field value from object
    constexpr const MemberType& get(const ClassType& obj) const
    {
        return obj.*memberPtr;
    }
    
    constexpr MemberType& get(ClassType& obj) const
    {
        return obj.*memberPtr;
    }
    
    // Check if field has a specific attribute
    template <typename Attr>
    constexpr bool hasAttribute() const
    {
        return (std::is_same_v<Attr, Attrs> || ...);
    }
    
    // Get attribute value (only if it exists)
    template <typename Attr>
    constexpr const Attr* getAttribute() const
    {
        if constexpr (hasAttribute<Attr>()) {
            return &std::get<Attr>(attributes);
        }
        return nullptr;
    }
    
    // Convenience: get props (returns None if no Props attribute)
    constexpr Prop getProps() const
    {
        if constexpr (hasAttribute<Props>()) {
            return std::get<Props>(attributes).value;
        }
        return Prop::None;
    }
    
    // Convenience getters for column names
    std::string_view getSqlColumn() const
    {
        if constexpr (has<SqlColumn>) {
            return std::get<SqlColumn>(attributes).name;
        } else {
            return fieldName;
        }
    }
    
    std::string_view getCsvColumn() const
    {
        if constexpr (has<CsvColumn>) {
            return std::get<CsvColumn>(attributes).name;
        } else {
            return fieldName;
        }
    }
    
    std::string_view getJsonProperty() const
    {
        if constexpr (has<JsonColumn>) {
            return std::get<JsonColumn>(attributes).name;
        } else {
            return fieldName;
        }
    }
};

// ============================================================================
// FIELD CREATION HELPER
// ============================================================================

template <auto MemberPtr, typename... Attrs>
constexpr auto field(const char* name, Attrs&&... attrs)
{
    return Field<MemberPtr, std::remove_cvref_t<Attrs>...>(name, std::forward<Attrs>(attrs)...);
}

// Alias for compatibility
template <auto MemberPtr, typename... Attrs>
constexpr auto MakeField(const char* name, Attrs&&... attrs)
{
    return field<MemberPtr>(name, std::forward<Attrs>(attrs)...);
}

} // namespace meta

