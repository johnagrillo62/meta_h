#pragma once
#include <string>
#include <sstream>
#include <string_view>

namespace meta {
    template <typename T>
    struct MetaTuple;
}

namespace meta {

// Helper to get SQL column name from field
template<typename FieldMeta>
std::string getFieldName(const FieldMeta& fieldMeta) {
    // Use getSqlColumn() helper from Field
    std::string_view sqlCol = fieldMeta.getSqlColumn();
    return std::string(sqlCol);
}

// Helper to get table name
template<typename T>
std::string getTableName() {
    if constexpr (requires { meta::MetaTuple<T>::tableName; }) {
        return std::string(meta::MetaTuple<T>::tableName);
    }
    return "";
}

// SQL type mapping
template<typename T>
std::string mapCppTypeToSQL() {
    using Type = std::remove_cv_t<std::remove_reference_t<T>>;
    
    if constexpr (std::is_same_v<Type, int>) return "INTEGER";
    else if constexpr (std::is_same_v<Type, int32_t>) return "INTEGER";
    else if constexpr (std::is_same_v<Type, int64_t>) return "BIGINT";
    else if constexpr (std::is_same_v<Type, std::string>) return "VARCHAR(255)";
    else if constexpr (std::is_same_v<Type, double>) return "DOUBLE";
    else if constexpr (std::is_same_v<Type, float>) return "FLOAT";
    else if constexpr (std::is_same_v<Type, bool>) return "BOOLEAN";
    else return "TEXT";
}

// CREATE TABLE
template<typename T>
std::string createTable() {
    std::string tableName = getTableName<T>();
    if (tableName.empty()) {
        throw std::runtime_error("Type must have tableName");
    }
    
    std::ostringstream sql;
    sql << "CREATE TABLE " << tableName << " (\n";
    
    const auto& fields = meta::MetaTuple<T>::FieldsMeta;
    bool first = true;
    
    std::apply([&](auto&&... field_metas) {
        auto addField = [&](const auto& fieldMeta) {
            if (!first) sql << ",\n";
            first = false;
            
            sql << "    " << getFieldName(fieldMeta) << " ";
            sql << mapCppTypeToSQL<typename std::decay_t<decltype(fieldMeta)>::MemberType>();
        };
        (addField(field_metas), ...);
    }, fields);
    
    sql << "\n);";
    return sql.str();
}

// INSERT
template<typename T>
std::string insertSQL(const T& obj) {
    std::string tableName = getTableName<T>();
    if (tableName.empty()) {
        throw std::runtime_error("Type must have tableName");
    }
    
    std::ostringstream sql;
    sql << "INSERT INTO " << tableName << " (";
    
    const auto& fields = meta::MetaTuple<T>::FieldsMeta;
    
    // Field names
    bool first = true;
    std::apply([&](auto&&... field_metas) {
        auto addName = [&](const auto& fieldMeta) {
            if (!first) sql << ", ";
            first = false;
            sql << getFieldName(fieldMeta);
        };
        (addName(field_metas), ...);
    }, fields);
    
    sql << ") VALUES (";
    
    // Values
    first = true;
    std::apply([&](auto&&... field_metas) {
        auto addValue = [&](const auto& fieldMeta) {
            if (!first) sql << ", ";
            first = false;
            
            const auto& value = obj.*(fieldMeta.memberPtr);
            
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>) {
                sql << "'" << value << "'";
            } else if constexpr (std::is_arithmetic_v<std::decay_t<decltype(value)>>) {
                sql << value;
            } else {
                sql << "NULL";
            }
        };
        (addValue(field_metas), ...);
    }, fields);
    
    sql << ")";
    return sql.str();
}

// SELECT
template<typename T>
std::string selectSQL() {
    std::string tableName = getTableName<T>();
    if (tableName.empty()) {
        throw std::runtime_error("Type must have tableName");
    }
    
    std::ostringstream sql;
    sql << "SELECT ";
    
    const auto& fields = meta::MetaTuple<T>::FieldsMeta;
    bool first = true;
    
    std::apply([&](auto&&... field_metas) {
        auto addField = [&](const auto& fieldMeta) {
            if (!first) sql << ", ";
            first = false;
            sql << getFieldName(fieldMeta);
        };
        (addField(field_metas), ...);
    }, fields);
    
    sql << " FROM " << tableName;
    return sql.str();
}

// UPDATE
template<typename T>
std::string updateSQL(const T& obj) {
    std::string tableName = getTableName<T>();
    if (tableName.empty()) {
        throw std::runtime_error("Type must have tableName");
    }
    
    std::ostringstream sql;
    sql << "UPDATE " << tableName << " SET ";
    
    const auto& fields = meta::MetaTuple<T>::FieldsMeta;
    bool first = true;
    std::string pkValue;
    
    std::apply([&](auto&&... field_metas) {
        auto addField = [&](const auto& fieldMeta) {
            const auto& value = obj.*(fieldMeta.memberPtr);
            
            if (first) {
                // Assume first field is primary key
                first = false;
                if constexpr (std::is_arithmetic_v<std::decay_t<decltype(value)>>) {
                    pkValue = std::to_string(value);
                }
                return;
            }
            
            if (!first) sql << ", ";
            sql << getFieldName(fieldMeta) << " = ";
            
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>) {
                sql << "'" << value << "'";
            } else if constexpr (std::is_arithmetic_v<std::decay_t<decltype(value)>>) {
                sql << value;
            }
        };
        (addField(field_metas), ...);
    }, fields);
    
    if (!pkValue.empty()) {
        sql << " WHERE id = " << pkValue;
    }
    
    return sql.str();
}

// DELETE
template<typename T>
std::string deleteSQL() {
    std::string tableName = getTableName<T>();
    if (tableName.empty()) {
        throw std::runtime_error("Type must have tableName");
    }
    return "DELETE FROM " + tableName + " WHERE id = ?";
}

} // namespace db

