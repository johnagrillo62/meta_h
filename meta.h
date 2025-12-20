// meta.h - Type-safe serialization using template overloading
// John A Grillo with assistance from Claude AI
//
// Just a fun way to experiment with reflection in C++.
// Collaborative effort with AI after many iterations to make an
// easy and fun to use reflection simulator. I had a basic understanding
// of templates, not enough to pull this off - AI helped get the ordering
// right and taught me how templates actually work.
//
// Supports YAML, JSON. No macros, compile-time type checking.
// Uses overload resolution, explicit specializations for primitives,
// and if constexpr for generic dispatch.

#pragma once

#include <array>
#include <deque>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace meta
{

// Enum support - register your enums by specializing EnumMapping
template <typename T> struct EnumMapping;

template <typename EnumT, auto& MappingArray> struct EnumTraitsAuto
{
    inline static constexpr auto& mapping = MappingArray;

    inline static const std::unordered_map<EnumT, std::string> enumToString = []()
    {
        std::unordered_map<EnumT, std::string> m;
        for (auto [e, s] : mapping)
            m[e] = s;
        return m;
    }();

    inline static const std::unordered_map<std::string, EnumT> stringToEnum = []()
    {
        std::unordered_map<std::string, EnumT> m;
        for (auto [e, s] : mapping)
            m[s] = e;
        return m;
    }();

    static std::string toString(EnumT e)
    {
        auto it = enumToString.find(e);
        return it != enumToString.end() ? it->second : "";
    }

    static std::optional<EnumT> fromString(const std::string& s)
    {
        auto it = stringToEnum.find(s);
        return it != stringToEnum.end() ? std::optional(it->second) : std::nullopt;
    }

    template <typename Func> static void forEach(Func f)
    {
        for (auto [e, _] : mapping)
            f(e);
    }
};

template <typename EnumT>
concept RegisteredEnum = requires { typename EnumMapping<EnumT>::Type; };

template <typename EnumT>
    requires RegisteredEnum<EnumT>
std::ostream& operator<<(std::ostream& os, EnumT e);

// Field metadata
enum class Requirement
{
    Required,
    Optional
};
constexpr auto OptionalField = Requirement::Optional;

template <typename MemberPtr> struct member_pointer_traits;
template <typename T, typename Class> struct member_pointer_traits<T Class::*>
{
    using type = T;
};

struct ValidationResult
{
    bool valid = true;
    std::vector<std::pair<std::string, std::string>> errors;
    
    void addError(std::string_view fieldName, std::string_view message)
    {
        valid = false;
        errors.push_back({std::string(fieldName), std::string(message)});
    }
};

template <auto MemberPtr> struct Field
{
    using member_type = typename member_pointer_traits<decltype(MemberPtr)>::type;
    std::string_view fieldName;
    std::string_view fieldDesc;
    Requirement requirement;
    static constexpr auto memberPtr = MemberPtr;

    constexpr Field(std::string_view name,
                    std::string_view desc = "",
                    Requirement req = Requirement::Required,
                    std::string_view annotate = "")
        : fieldName(name), fieldDesc(desc), requirement(req)
    {
    }
};

// Abstract interfaces for format-agnostic serialization
struct Node
{
    virtual ~Node() = default;
    virtual std::optional<int> asInt() const = 0;
    virtual std::optional<bool> asBool() const = 0;
    virtual std::optional<std::string> asString() const = 0;
    virtual bool isSequence() const = 0;
    virtual bool isMap() const = 0;
    virtual bool isNull() const = 0;
    virtual size_t size() const = 0;
    virtual std::unique_ptr<Node> at(size_t i) const = 0;
    virtual std::unique_ptr<Node> at(std::string k) const = 0;
    virtual std::vector<std::string> keys() const = 0;
};

struct Builder
{
    virtual ~Builder() = default;
    virtual void writeInt(int v) = 0;
    virtual void writeDouble(double v) = 0;
    virtual void writeBool(bool v) = 0;
    virtual void writeString(const std::string& v) = 0;
    virtual void startSeq(const std::string& elemType = "") = 0;
    virtual void endSeq() = 0;
    virtual void startFlowSeq() { startSeq(""); }
    virtual void endFlowSeq() { endSeq(); }
    virtual void startMap(const std::string& valueType = "") = 0;
    virtual void endMap() = 0;
    virtual void key(const std::string& k) = 0;
    virtual std::string result() = 0;
};

// YAML implementation
class YamlNode : public Node
{
    YAML::Node node;

  public:
    YamlNode(YAML::Node n) : node(n) {}

    std::optional<int> asInt() const override
    {
        try { return node.as<int>(); }
        catch (...) { return {}; }
    }

    std::optional<bool> asBool() const
    {
        try { return node.as<bool>(); }
        catch (...) { return {}; }
    }

    std::optional<std::string> asString() const override
    {
        try { return node.as<std::string>(); }
        catch (...) { return {}; }
    }

    bool isSequence() const override { return node.IsSequence(); }
    bool isMap() const override { return node.IsMap(); }
    bool isNull() const override { return node.IsNull(); }
    size_t size() const override { return node.size(); }

    std::unique_ptr<Node> at(size_t i) const override
    {
        return std::make_unique<YamlNode>(node[i]);
    }

    std::unique_ptr<Node> at(std::string k) const override
    {
        if (!node[k])
            return nullptr;
        return std::make_unique<YamlNode>(node[k]);
    }

    std::vector<std::string> keys() const override
    {
        std::vector<std::string> k;
        for (auto& item : node)
            if (item.first.IsScalar())
                k.push_back(item.first.as<std::string>());
        return k;
    }
};

class YamlBuilder : public Builder
{
    std::ostringstream out;
    int indent = 0;
    bool inSequence = false;
    bool inFlowSeq = false;
    bool firstInFlow = true;

  public:
    void writeInt(int v) override
    {
        if (inFlowSeq && !firstInFlow)
            out << ", ";
        else if (inSequence && !inFlowSeq)
            out << "\n" << std::string(indent * 2, ' ') << "- ";
        out << v;
        if (inFlowSeq)
            firstInFlow = false;
    }

    void writeDouble(double v) override
    {
        if (inFlowSeq && !firstInFlow)
            out << ", ";
        else if (inSequence && !inFlowSeq)
            out << "\n" << std::string(indent * 2, ' ') << "- ";
        out << v;
        if (inFlowSeq)
            firstInFlow = false;
    }

    void writeBool(bool v) override
    {
        if (inFlowSeq && !firstInFlow)
            out << ", ";
        else if (inSequence && !inFlowSeq)
            out << "\n" << std::string(indent * 2, ' ') << "- ";
        out << (v ? "true" : "false");
        if (inFlowSeq)
            firstInFlow = false;
    }

    void writeString(const std::string& v) override
    {
        if (inFlowSeq && !firstInFlow)
            out << ", ";
        else if (inSequence && !inFlowSeq)
            out << "\n" << std::string(indent * 2, ' ') << "- ";
        out << v;
        if (inFlowSeq)
            firstInFlow = false;
    }

    void startSeq(const std::string& elemType = "") override
    {
        if (!inFlowSeq)
        {
            indent++;
            inSequence = true;
            if (!elemType.empty())
                out << "  # seq<" << elemType << ">";
        }
    }

    void endSeq() override
    {
        if (!inFlowSeq)
        {
            indent--;
            inSequence = false;
        }
    }

    void startFlowSeq() override
    {
        inFlowSeq = true;
        firstInFlow = true;
        out << "[";
    }

    void endFlowSeq() override
    {
        out << "]";
        inFlowSeq = false;
    }

    void startMap(const std::string& valueType = "") override
    {
        if (!inFlowSeq)
        {
            indent++;
            if (!valueType.empty())
                out << "  # map<string," << valueType << ">";
        }
    }

    void endMap() override
    {
        if (!inFlowSeq)
            indent--;
    }

    void key(const std::string& k) override
    {
        if (!inFlowSeq)
        {
            if (!out.str().empty())
                out << "\n";
            out << std::string(indent * 2, ' ') << k << ": ";
        }
    }

    std::string result() override { return out.str(); }
};

class JsonBuilder : public Builder
{
    std::ostringstream out;
    bool needsComma = false;

    void comma()
    {
        if (needsComma)
            out << ",";
        needsComma = true;
    }

  public:
    void writeInt(int v) override { comma(); out << v; }
    void writeDouble(double v) override { comma(); out << v; }
    void writeBool(bool v) override { comma(); out << (v ? "true" : "false"); }
    void writeString(const std::string& v) override { comma(); out << "\"" << v << "\""; }

    void startSeq(const std::string& = "") override
    {
        if (needsComma) out << ",";
        out << "[";
        needsComma = false;
    }

    void endSeq() override { out << "]"; needsComma = true; }

    void startMap(const std::string& = "") override
    {
        if (needsComma) out << ",";
        out << "{";
        needsComma = false;
    }

    void endMap() override { out << "}"; needsComma = true; }

    void key(const std::string& k) override
    {
        if (needsComma) out << ",";
        out << "\"" << k << "\":";
        needsComma = false;
    }

    std::string result() override { return out.str(); }
};

// Forward declarations
template <typename K, typename V>
inline ValidationResult from(std::pair<K, V>& obj, Node* node);
template <typename... Args>
inline ValidationResult from(std::tuple<Args...>& obj, Node* node);
template <typename T> 
inline ValidationResult from(std::vector<T>& obj, Node* node);
template <typename K, typename V>
inline ValidationResult from(std::map<K, V>& obj, Node* node);
template <typename T> 
inline ValidationResult from(std::optional<T>& obj, Node* node);

template <typename T> inline ValidationResult from(T& obj, Node* node);

// Enum deserialization
template <RegisteredEnum EnumT> 
inline ValidationResult from(EnumT& obj, Node* node)
{
    auto s = node->asString();
    if (!s)
    {
        ValidationResult r;
        r.addError("", "Invalid enum");
        return r;
    }
    auto e = EnumMapping<EnumT>::Type::fromString(*s);
    if (!e)
    {
        ValidationResult r;
        r.addError("", "Unknown enum value: " + *s);
        return r;
    }
    obj = *e;
    return ValidationResult();
}

template <typename K, typename V>
inline ValidationResult fromPair(std::pair<K, V>& obj, Node* node)
{
    ValidationResult result;
    if (!node->isSequence() || node->size() != 2)
    {
        result.addError("", "Expected sequence of 2 elements, got size=" + std::to_string(node->size()));
        return result;
    }
    
    auto kNode = node->at(0);
    auto vNode = node->at(1);
    if (!kNode || !vNode)
    {
        result.addError("", "Failed to get pair elements");
        return result;
    }

    auto kResult = from(obj.first, kNode.get());
    auto vResult = from(obj.second, vNode.get());

    if (!kResult.valid)
        for (auto& [f, e] : kResult.errors)
            result.addError("[0]" + (f.empty() ? "" : "." + f), e);
    
    if (!vResult.valid)
        for (auto& [f, e] : vResult.errors)
            result.addError("[1]" + (f.empty() ? "" : "." + f), e);
    
    if (kResult.valid && vResult.valid)
    {
        result.valid = true;
        result.errors.clear();
    }
    return result;
}

template <typename K, typename V>
inline ValidationResult from(std::pair<K, V>& obj, Node* node)
{
    return fromPair(obj, node);
}

// Container helpers
template <typename... Args>
inline ValidationResult fromVector(std::vector<std::tuple<Args...>>& obj, Node* node);

template <typename K, typename V>
inline ValidationResult fromVector(std::vector<std::pair<K, V>>& obj, Node* node);

template <typename... Args>
inline ValidationResult fromMap(std::map<std::string, std::tuple<Args...>>& obj, Node* node);

template <typename T> 
inline ValidationResult fromVector(std::vector<T>& obj, Node* node)
{
    ValidationResult result;
    if (!node->isSequence())
    {
        result.addError("", "Expected sequence");
        return result;
    }
    
    obj.clear();
    for (size_t i = 0; i < node->size(); ++i)
    {
        T elem{};
        auto elemNode = node->at(i);
        auto elemResult = from(elem, elemNode.get());
        if (!elemResult.valid)
        {
            for (auto& [f, e] : elemResult.errors)
                result.addError("[" + std::to_string(i) + "]" + (f.empty() ? "" : "." + f), e);
        }
        else
            obj.push_back(elem);
    }
    return result;
}

template <typename K, typename V> 
inline ValidationResult fromMap(std::map<K, V>& obj, Node* node)
{
    ValidationResult result;
    if (!node->isMap())
    {
        result.addError("", "Expected map");
        return result;
    }
    
    obj.clear();
    for (auto& k : node->keys())
    {
        V v{};
        auto vNode = node->at(k);
        if (!vNode) continue;
        
        auto vResult = from(v, vNode.get());
        if (!vResult.valid)
        {
            for (auto& [f, e] : vResult.errors)
                result.addError(k + (f.empty() ? "" : "." + f), e);
        }
        else
            obj[k] = v;
    }
    return result;
}

template <typename T> 
inline ValidationResult fromOptional(std::optional<T>& obj, Node* node)
{
    if (node->isNull())
    {
        obj = std::nullopt;
        return ValidationResult();
    }
    T v{};
    auto result = from(v, node);
    if (result.valid)
        obj = v;
    return result;
}

template <typename T> 
inline ValidationResult from(std::vector<T>& obj, Node* node)
{
    return fromVector(obj, node);
}

template <typename K, typename V>
inline ValidationResult from(std::map<K, V>& obj, Node* node)
{
    return fromMap(obj, node);
}

template <typename T> 
inline ValidationResult from(std::optional<T>& obj, Node* node)
{
    return fromOptional(obj, node);
}

// Tuple support
template <typename... Args> 
inline ValidationResult fromTuple(std::tuple<Args...>& obj, Node* node)
{
    ValidationResult result;
    if (!node->isSequence() || node->size() != sizeof...(Args))
    {
        result.addError("", "Expected sequence of " + std::to_string(sizeof...(Args)) +
                            " elements, got " + std::to_string(node->size()));
        return result;
    }

    fromTupleElements<0, Args...>(obj, node, result);

    if (result.errors.empty())
        result.valid = true;
    return result;
}

template <size_t I, typename... Args>
void fromTupleElements(std::tuple<Args...>& obj, Node* node, ValidationResult& result)
{
    if constexpr (I < sizeof...(Args))
    {
        auto elemNode = node->at(I);
        if (elemNode)
        {
            auto elemResult = from(std::get<I>(obj), elemNode.get());
            if (!elemResult.valid)
                for (auto& [f, e] : elemResult.errors)
                    result.addError("[" + std::to_string(I) + "]" + (f.empty() ? "" : "." + f), e);
        }
        else
            result.addError("[" + std::to_string(I) + "]", "Failed to get element");
        
        fromTupleElements<I + 1, Args...>(obj, node, result);
    }
}

template <typename... Args>
inline ValidationResult from(std::tuple<Args...>& obj, Node* node)
{
    return fromTuple(obj, node);
}

template <typename... Args>
inline ValidationResult fromVector(std::vector<std::tuple<Args...>>& obj, Node* node)
{
    ValidationResult result;
    if (!node->isSequence())
    {
        result.addError("", "Expected sequence");
        return result;
    }
    
    obj.clear();
    for (size_t i = 0; i < node->size(); ++i)
    {
        std::tuple<Args...> elem{};
        auto elemNode = node->at(i);
        auto elemResult = fromTuple(elem, elemNode.get());
        if (!elemResult.valid)
        {
            for (auto& [f, e] : elemResult.errors)
                result.addError("[" + std::to_string(i) + "]" + (f.empty() ? "" : "." + f), e);
        }
        else
            obj.push_back(elem);
    }
    return result;
}

template <typename K, typename V>
inline ValidationResult fromVector(std::vector<std::pair<K, V>>& obj, Node* node)
{
    ValidationResult result;
    if (!node->isSequence())
    {
        result.addError("", "Expected sequence");
        return result;
    }
    
    obj.clear();
    for (size_t i = 0; i < node->size(); ++i)
    {
        std::pair<K, V> elem{};
        auto elemNode = node->at(i);
        auto elemResult = fromPair(elem, elemNode.get());
        if (!elemResult.valid)
        {
            for (auto& [f, e] : elemResult.errors)
                result.addError("[" + std::to_string(i) + "]" + (f.empty() ? "" : "." + f), e);
        }
        else
            obj.push_back(elem);
    }
    return result;
}

template <typename... Args>
inline ValidationResult fromMap(std::map<std::string, std::tuple<Args...>>& obj, Node* node)
{
    ValidationResult result;
    if (!node->isMap())
    {
        result.addError("", "Expected map");
        return result;
    }
    
    obj.clear();
    for (auto& k : node->keys())
    {
        std::tuple<Args...> v{};
        auto vNode = node->at(k);
        if (!vNode) continue;
        
        auto vResult = fromTuple(v, vNode.get());
        if (!vResult.valid)
        {
            for (auto& [f, e] : vResult.errors)
                result.addError(k + (f.empty() ? "" : "." + f), e);
        }
        else
            obj[k] = v;
    }
    return result;
}

template <typename T> inline ValidationResult fromStruct(T& obj, Node* node);

// Primitive specializations
template <> inline ValidationResult from(int& obj, Node* node)
{
    auto v = node->asInt();
    if (!v)
    {
        ValidationResult r;
        r.addError("", "Invalid integer");
        return r;
    }
    obj = *v;
    return ValidationResult();
}

template <> inline ValidationResult from(double& obj, Node* node)
{
    auto v = node->asInt();
    if (v)
        obj = *v;
    return ValidationResult();
}

template <> inline ValidationResult from<std::string>(std::string& obj, Node* node)
{
    auto v = node->asString();
    if (!v)
    {
        ValidationResult r;
        r.addError("", "Invalid string");
        return r;
    }
    obj = *v;
    return ValidationResult();
}

template <> inline ValidationResult from(bool& obj, Node* node)
{
    auto v = node->asBool();
    if (!v)
    {
        ValidationResult r;
        r.addError("", "Invalid boolean");
        return r;
    }
    obj = *v;
    return ValidationResult();
}

// Generic dispatcher
template <typename T> inline ValidationResult from(T& obj, Node* node)
{
    if constexpr (requires { T::fields; })
        return fromStruct<T>(obj, node);
    else if constexpr (requires { typename T::Deser; })
        return T::Deser::parse(obj, node);
    else
    {
        static_assert(requires { T::fields; } || requires { typename T::Deser; },
                      "Type must have either 'fields' tuple or 'Deser' struct");
        return ValidationResult();
    }
}

template <typename T> inline ValidationResult fromStruct(T& obj, Node* node)
{
    ValidationResult result;
    std::apply(
        [&](auto&&... fields)
        {
            (..., [&](auto& field)
             {
                 auto fieldNode = node->at(std::string(field.fieldName));
                 if (!fieldNode)
                 {
                     if (field.requirement == Requirement::Required)
                         result.addError(field.fieldName, "Missing required field");
                     return;
                 }
                 auto fieldResult = from(obj.*(field.memberPtr), fieldNode.get());
                 if (!fieldResult.valid)
                     for (auto& [f, e] : fieldResult.errors)
                         result.addError(std::string(field.fieldName) + (f.empty() ? "" : "." + f), e);
             }(fields));
        },
        T::fields);
    return result;
}

// Serialization
inline void to(const int& obj, Builder* b);
inline void to(const double& obj, Builder* b);
inline void to(const bool& obj, Builder* b);
inline void to(const std::string& obj, Builder* b);

template <typename T> inline void to(const T& obj, Builder* b)
{
    if constexpr (requires { typename T::Ser; })
    {
        T::Ser::write(obj, b);
    }
    else if constexpr (requires { T::fields; })
    {
        b->startMap("");
        std::apply(
            [&](auto&&... fields)
            {
                (..., [&](auto& field)
                 {
                     b->key(std::string(field.fieldName));
                     to(obj.*(field.memberPtr), b);
                 }(fields));
            },
            T::fields);
        b->endMap();
    }
}

inline void to(const int& obj, Builder* b) { b->writeInt(obj); }
inline void to(const double& obj, Builder* b) { b->writeDouble(obj); }
inline void to(const bool& obj, Builder* b) { b->writeBool(obj); }
inline void to(const std::string& obj, Builder* b) { b->writeString(obj); }

template <RegisteredEnum EnumT> 
inline void to(const EnumT& obj, Builder* b)
{
    b->writeString(EnumMapping<EnumT>::Type::toString(obj));
}

template <typename T> void toVector(const std::vector<T>& v, Builder* b)
{
    b->startSeq(typeid(T).name());
    for (const auto& e : v)
        to(e, b);
    b->endSeq();
}

template <typename K, typename V> void toMap(const std::map<K, V>& m, Builder* b)
{
    b->startMap(typeid(V).name());
    for (const auto& [k, v] : m)
    {
        b->key(k);
        to(v, b);
    }
    b->endMap();
}

template <typename T> void toOptional(const std::optional<T>& opt, Builder* b)
{
    if (opt)
        to(*opt, b);
    else
        b->writeString("null");
}

template <typename K, typename V> void toPair(const std::pair<K, V>& p, Builder* b)
{
    b->startFlowSeq();
    to(p.first, b);
    to(p.second, b);
    b->endFlowSeq();
}

template <typename K, typename V> 
inline void to(const std::pair<K, V>& obj, Builder* b)
{
    toPair(obj, b);
}

template <typename... Args> void toTuple(const std::tuple<Args...>& tup, Builder* b)
{
    b->startFlowSeq();
    std::apply([b](const auto&... args)
               { (..., [b](const auto& arg) { to(arg, b); }(args)); },
               tup);
    b->endFlowSeq();
}

template <typename... Args> 
inline void to(const std::tuple<Args...>& obj, Builder* b)
{
    toTuple(obj, b);
}

template <typename T> 
inline void to(const std::vector<T>& obj, Builder* b)
{
    toVector(obj, b);
}

template <typename K, typename V> 
inline void to(const std::map<K, V>& obj, Builder* b)
{
    toMap(obj, b);
}

template <typename T> 
inline void to(const std::optional<T>& obj, Builder* b)
{
    toOptional(obj, b);
}

// Filesystem path support
template <> inline ValidationResult from(std::filesystem::path& obj, Node* node)
{
    auto str = node->asString();
    if (!str)
    {
        ValidationResult r;
        r.addError("path", "Must be string");
        return r;
    }
    obj = std::filesystem::path(*str);
    return ValidationResult();
}

template <> inline void to(const std::filesystem::path& obj, Builder* b)
{
    b->writeString(obj.string());
}

// Public API
template <typename T>
std::pair<std::optional<T>, ValidationResult> reifyFromYaml(const std::string& yaml)
{
    try
    {
        YAML::Node node = YAML::Load(yaml);
        YamlNode ynode(node);
        T obj{};
        auto result = from(obj, &ynode);
        return result.valid ? std::make_pair(std::optional(obj), result)
                            : std::make_pair(std::nullopt, result);
    }
    catch (const std::exception& e)
    {
        ValidationResult result;
        result.addError("yaml", std::string(e.what()));
        return {std::nullopt, result};
    }
}

template <typename T>
std::pair<std::optional<T>, ValidationResult> reifyFromYaml(const YAML::Node& node)
{
    try
    {
        YamlNode ynode(node);
        T obj{};
        auto result = from(obj, &ynode);
        return result.valid ? std::make_pair(std::optional(obj), result)
                            : std::make_pair(std::nullopt, result);
    }
    catch (const std::exception& e)
    {
        ValidationResult result;
        result.addError("yaml", std::string(e.what()));
        return {std::nullopt, result};
    }
}

template <typename T> std::string toYaml(const T& obj)
{
    YamlBuilder builder;
    to(obj, &builder);
    return builder.result();
}

template <typename T> std::string toJson(const T& obj)
{
    JsonBuilder builder;
    to(obj, &builder);
    return builder.result();
}

template <typename T> std::string toString(const T& obj)
{
    return toYaml(obj);
}

template <typename T> bool checkForEquality(const T& a, const T& b)
{
    if constexpr (requires { T::fields; })
    {
        bool equal = true;
        std::apply(
            [&](auto&&... fields)
            {
                (..., [&](auto& field)
                 {
                     if (a.*(field.memberPtr) != b.*(field.memberPtr))
                         equal = false;
                 }(fields));
            },
            T::fields);
        return equal;
    }
    else
        return a == b;
}

template <typename EnumT>
    requires RegisteredEnum<EnumT>
std::ostream& operator<<(std::ostream& os, EnumT e)
{
    os << EnumMapping<EnumT>::Type::toString(e);
    return os;
}

template <typename EnumT>
    requires RegisteredEnum<EnumT>
std::optional<EnumT> toEnum(const std::string& s)
{
    return EnumMapping<EnumT>::Type::fromString(s);
}

template <typename EnumT>
    requires RegisteredEnum<EnumT>
std::vector<EnumT> enumValues()
{
    std::vector<EnumT> values;
    EnumMapping<EnumT>::Type::forEach([&](EnumT e) { values.push_back(e); });
    return values;
}

} // namespace meta

template <typename EnumT>
    requires meta::RegisteredEnum<EnumT>
std::ostream& operator<<(std::ostream& os, EnumT e)
{
    os << meta::EnumMapping<EnumT>::Type::toString(e);
    return os;
}

