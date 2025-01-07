struct Range {
    int lower;
    int upper;
    
    bool contains(int value) const {
        return value >= lower && value <= upper;
    }
    
    bool overlaps(const Range& other) const {
        return !(other.upper < lower || other.lower > upper);
    }
};

struct RangeKey {
    string tableName;
    string columnName;
    Range range;
    bool isWrite;  // 是否为写操作
    
    RangeKey(string table, string column, Range r, bool write) 
        : tableName(table), columnName(column), range(r), isWrite(write) {}
}; 