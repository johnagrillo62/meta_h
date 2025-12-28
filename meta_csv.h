/*
 * meta_csv.h - CSV Serialization Extension for meta.h
 *
 * Provides CSV serialization for structs using the meta.h reflection framework.
 * Supports:
 * - Basic types (int, double, bool, string)
 * - Nested structures (flattened to key=value pairs)
 * - Vectors (inline with semicolon separators)
 * - CSV header generation from field metadata
 * - Batch serialization from vector of objects
 *
 * Usage:
 *   #include "meta.h"
 *   #include "meta_csv.h"
 *
 *   struct Person { ... };
 *   std::vector<Person> people = {...};
 *   
 *   std::string csv = meta::serialize(people);
 *   std::string csv = meta::serializeAdvanced(people, ",", true, true);
 */

#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <type_traits>

// Forward declare if meta.h not included yet
namespace meta { class Builder; }

#include "meta.h"

namespace meta
{

// ============================================================================
// CSV BUILDER - Implements Builder interface for CSV output
// ============================================================================

class CSVBuilder : public Builder
{
  private:
    std::ostringstream out;
    int mapDepth = 0;
    bool firstInCurrentLevel = true;
    std::vector<bool> isActualMap; // Track if each level is actual map vs struct

  public:
    void writeInt(int v) override
    {
        outputSeparator();
        out << v;
    }

    void writeDouble(double v) override
    {
        outputSeparator();
        out << v;
    }

    void writeBool(bool v) override
    {
        outputSeparator();
        out << (v ? "true" : "false");
    }

    void writeString(const std::string& v) override
    {
        outputSeparator();
        // Escape quotes by doubling them (CSV standard)
        std::string escaped = v;
        size_t pos = 0;
        while ((pos = escaped.find('"', pos)) != std::string::npos)
        {
            escaped.replace(pos, 1, "\"\"");
            pos += 2;
        }
        out << "\"" << escaped << "\"";
    }

    void writeNull() override
    {
        outputSeparator();
        // Empty field for null
    }

    void startSeq(const std::string& = "") override
    {
        // Sequences inline - bracket notation
        out << "[";
    }

    void endSeq() override
    {
        out << "]";
    }

    void startFlowSeq() override
    {
        out << "[";
    }

    void endFlowSeq() override
    {
        out << "]";
    }

    void startMap(const std::string& = "") override
    {
        mapDepth++;
        isActualMap.push_back(false); // Assume struct unless we see actual key=value pairs
        firstInCurrentLevel = true;
        
        if (mapDepth > 1)
            out << "{";
    }

    void endMap() override
    {
        if (mapDepth > 1)
            out << "}";
        mapDepth--;
        isActualMap.pop_back();
    }

    void key(const std::string& k) override
    {
        // Top-level struct fields: comma-separated
        if (mapDepth == 1)
        {
            if (!firstInCurrentLevel)
                out << ",";
            firstInCurrentLevel = false;
        }
        // Nested maps: semicolon-separated key=value pairs
        else if (mapDepth > 1)
        {
            isActualMap[mapDepth - 1] = true;
            if (!firstInCurrentLevel)
                out << ";";
            
            std::string escaped = k;
            size_t pos = 0;
            while ((pos = escaped.find('"', pos)) != std::string::npos)
            {
                escaped.replace(pos, 1, "\"\"");
                pos += 2;
            }
            out << escaped << "=";
            firstInCurrentLevel = false;
        }
    }

    std::string result() override
    {
        return out.str();
    }

  private:
    void outputSeparator()
    {
        // Separators are handled by key() for maps
        // For sequences, no separator needed (handled by startSeq/endSeq)
    }
};

// ============================================================================
// PUBLIC API - toCSV using the Builder pattern
// ============================================================================

template <typename T> 
std::string toCSV(const T& obj)
{
    CSVBuilder builder;
    to(obj, &builder);
    return builder.result();
}

// ============================================================================
// CSV HEADER GENERATION - from fields metadata
// ============================================================================

template <HasFields T>
std::string toCSVHeader()
{
    std::ostringstream oss;
    bool first = true;
    
    std::apply(
        [&](auto&&... fields)
        {
            (...,
             [&](auto& field)
             {
                 if (!first)
                     oss << ",";
                 
                 // Escape field name
                 std::string escaped = std::string(field.fieldName);
                 size_t pos = 0;
                 while ((pos = escaped.find('"', pos)) != std::string::npos)
                 {
                     escaped.replace(pos, 1, "\"\"");
                     pos += 2;
                 }
                 oss << "\"" << escaped << "\"";
                 first = false;
             }(fields));
        },
        get_fields<T>());
        
    return oss.str();
}

// ============================================================================
// BATCH CSV EXPORT - Multiple objects with header
// ============================================================================

template <HasFields T>
std::string toCSVWithHeader(const std::vector<T>& objects)
{
    std::ostringstream oss;
    
    // Header row
    oss << toCSVHeader<T>() << "\n";
    
    // Data rows
    for (const auto& obj : objects)
    {
        oss << toCSV(obj) << "\n";
    }
    
    return oss.str();
}

// ============================================================================
// ADVANCED CSV SERIALIZATION - Direct field-based serialization
// ============================================================================

// Helper: Check if field should be skipped
template <typename FieldType>
constexpr bool shouldSkipField(const FieldType& field)
{
    // Skip fields with null member pointers (private/inaccessible fields)
    if constexpr (requires { field.memberPtr; })
    {
        return field.memberPtr == nullptr;
    }
    return false;
}

// Helper: Get CSV column name from field
template <typename FieldType>
std::string getCSVColumnName(const FieldType& field)
{
    // Use field name directly
    return std::string(field.fieldName);
}

// Helper: Write CSV value with proper escaping
template <typename T>
void writeCSVValue(std::ostringstream& os, const T& value)
{
    if constexpr (std::is_same_v<T, std::string>)
    {
        // Escape quotes and wrap in quotes
        std::string escaped = value;
        size_t pos = 0;
        while ((pos = escaped.find('"', pos)) != std::string::npos)
        {
            escaped.replace(pos, 1, "\"\"");
            pos += 2;
        }
        os << "\"" << escaped << "\"";
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        os << (value ? "true" : "false");
    }
    else if constexpr (std::is_arithmetic_v<T>)
    {
        os << value;
    }
    else
    {
        // For complex types, use Builder-based serialization
        CSVBuilder builder;
        to(value, &builder);
        os << "\"" << builder.result() << "\"";
    }
}

// ============================================================================
// Main CSV serialization function from vector
// ============================================================================

template <HasFields ObjectType>
std::string serialize(const std::vector<ObjectType>& objects, const std::string& delimiter = ",")
{
    std::ostringstream os;
    if (objects.empty())
    {
        return "";
    }
    
    const auto& fields = get_fields<ObjectType>();
    
    // Write header
    std::apply([&](auto&&... fieldMeta)
    {
        bool first = true;
        auto writeHeader = [&](const auto& field)
        {
            if (!shouldSkipField(field))
            {
                if (!first) os << delimiter;
                first = false;
                os << getCSVColumnName(field);
            }
        };
        (writeHeader(fieldMeta), ...);
        os << "\n";
    }, fields);
    
    // Write data rows
    for (const auto& obj : objects)
    {
        std::apply([&](auto&&... fieldMeta)
        {
            bool first = true;
            auto writeData = [&](const auto& field)
            {
                if (!shouldSkipField(field))
                {
                    if (!first) os << delimiter;
                    first = false;
                    
                    if constexpr (requires { field.memberPtr; })
                    {
                        if constexpr (field.memberPtr != nullptr)
                        {
                            auto value = obj.*(field.memberPtr);
                            writeCSVValue(os, value);
                        }
                    }
                }
            };
            (writeData(fieldMeta), ...);
            os << "\n";
        }, fields);
    }
    
    return os.str();
}

// ============================================================================
// Advanced CSV serialization with more options
// ============================================================================

template <HasFields ObjectType>
std::string serializeAdvanced(const std::vector<ObjectType>& objects, 
                              const std::string& delimiter = ",",
                              bool includeHeader = true,
                              bool escapeStrings = true)
{
    std::ostringstream os;
    if (objects.empty())
    {
        return "";
    }
    
    const auto& fields = get_fields<ObjectType>();
    
    // Write header if requested
    if (includeHeader)
    {
        std::apply([&](auto&&... fieldMeta)
        {
            bool first = true;
            auto writeHeader = [&](const auto& field)
            {
                if (!shouldSkipField(field))
                {
                    if (!first) os << delimiter;
                    first = false;
                    os << getCSVColumnName(field);
                }
            };
            (writeHeader(fieldMeta), ...);
            os << "\n";
        }, fields);
    }
    
    // Write data rows
    for (const auto& obj : objects)
    {
        std::apply([&](auto&&... fieldMeta)
        {
            bool first = true;
            auto writeData = [&](const auto& field)
            {
                if (!shouldSkipField(field))
                {
                    if (!first) os << delimiter;
                    first = false;
                    
                    if constexpr (requires { field.memberPtr; })
                    {
                        if constexpr (field.memberPtr != nullptr)
                        {
                            auto value = obj.*(field.memberPtr);
                            if (escapeStrings)
                            {
                                writeCSVValue(os, value);
                            }
                            else
                            {
                                os << value;
                            }
                        }
                    }
                }
            };
            (writeData(fieldMeta), ...);
            os << "\n";
        }, fields);
    }
    
    return os.str();
}

// ============================================================================
// Get CSV headers as vector of strings
// ============================================================================

template <HasFields ObjectType>
std::vector<std::string> getHeaders()
{
    std::vector<std::string> headers;
    const auto& fields = get_fields<ObjectType>();
    
    std::apply([&](auto&&... fieldMeta)
    {
        auto addHeader = [&](const auto& field)
        {
            if (!shouldSkipField(field))
            {
                headers.push_back(getCSVColumnName(field));
            }
        };
        (addHeader(fieldMeta), ...);
    }, fields);
    
    return headers;
}

} // namespace meta

