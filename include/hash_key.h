#ifndef HASH_KEY_H
#define HASH_KEY_H

#include <string>
#include "range.h"

struct HashKey {
    std::string tableName;
    std::string columnName;
    Range predicate;
    
    HashKey(std::string table, std::string column, Range pred)
        : tableName(table), columnName(column), predicate(pred) {}
    
    bool operator==(const HashKey& other) const {
        return tableName == other.tableName && 
               columnName == other.columnName && 
               predicate.lower == other.predicate.lower &&
               predicate.upper == other.predicate.upper;
    }
};

namespace std {
    template<>
    struct hash<HashKey> {
        size_t operator()(const HashKey& k) const {
            return hash<string>()(k.tableName) ^ 
                   hash<string>()(k.columnName) ^ 
                   hash<int>()(k.predicate.lower) ^
                   hash<int>()(k.predicate.upper);
        }
    };
}

#endif // HASH_KEY_H 