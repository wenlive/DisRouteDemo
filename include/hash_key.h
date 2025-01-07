struct HashKey {
    string tableName;
    string columnName;
    Range predicate;
    
    bool operator==(const HashKey& other) const {
        return tableName == other.tableName && 
               columnName == other.columnName && 
               predicate.lower == other.predicate.lower &&
               predicate.upper == other.predicate.upper;
    }
};

// 为了使用HashKey作为map的key
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