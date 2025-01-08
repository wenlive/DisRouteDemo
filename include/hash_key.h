#ifndef HASH_KEY_H
#define HASH_KEY_H

#include <string>
#include <unordered_map>
#include "range.h"

struct ColumnRange {
    std::string columnName;
    Range range;
    
    ColumnRange(const std::string& col, const Range& r) 
        : columnName(col), range(r) {}
};

struct HashKey {
    std::string tableName;
    std::vector<ColumnRange> columnRanges;  // 支持多列范围
    
    HashKey(std::string table, const std::vector<ColumnRange>& ranges)
        : tableName(table), columnRanges(ranges) {}
    
    bool operator==(const HashKey& other) const {
        if (tableName != other.tableName || 
            columnRanges.size() != other.columnRanges.size())
            return false;
            
        for (size_t i = 0; i < columnRanges.size(); i++) {
            if (columnRanges[i].columnName != other.columnRanges[i].columnName ||
                columnRanges[i].range.lower != other.columnRanges[i].range.lower ||
                columnRanges[i].range.upper != other.columnRanges[i].range.upper)
                return false;
        }
        return true;
    }
};

namespace std {
    template<>
    struct hash<HashKey> {
        size_t operator()(const HashKey& k) const {
            size_t h = hash<string>()(k.tableName);
            for (const auto& col : k.columnRanges) {
                h ^= hash<string>()(col.columnName) ^
                     hash<int>()(col.range.lower) ^
                     hash<int>()(col.range.upper);
            }
            return h;
        }
    };
}

#endif // HASH_KEY_H 