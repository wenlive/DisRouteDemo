#ifndef RANGE_H
#define RANGE_H

#include <string>

struct Range {
    int lower;
    int upper;
    
    Range(int l, int u) : lower(l), upper(u) {}
    
    bool contains(int value) const {
        return value >= lower && value <= upper;
    }
    
    bool overlaps(const Range& other) const {
        return !(other.upper < lower || other.lower > upper);
    }
};

struct RangeKey {
    std::string tableName;
    std::string columnName;
    Range range;
    bool isWrite;
    
    RangeKey(std::string table, std::string column, Range r, bool write) 
        : tableName(table), columnName(column), range(r), isWrite(write) {}
};

#endif // RANGE_H 