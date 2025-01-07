class QueryRouter {
private:
    unordered_map<HashKey, int> routingTable;  // 路由表，映射到nodeId
    const int nodeCount;
    
    // 模拟的数据分布情况
    vector<unordered_map<string, vector<Range>>> nodeDataRanges;

public:
    QueryRouter(int nodes) : nodeCount(nodes) {
        nodeDataRanges.resize(nodes);
    }
    
    // 初始化数据分布
    void initializeDataDistribution(const vector<pair<string, vector<Range>>>& distribution) {
        // 根据预设的亲和性分布初始化路由表
        for (int nodeId = 0; nodeId < nodeCount; nodeId++) {
            for (const auto& [table, ranges] : distribution) {
                for (const Range& range : ranges) {
                    HashKey key{table, "id", range};  // 简化版本只考虑id列
                    routingTable[key] = nodeId;
                }
            }
        }
    }
    
    // 路由决策
    int route(const vector<RangeKey>& queryRanges) {
        vector<int> votes(nodeCount, 0);
        
        for (const auto& range : queryRanges) {
            // 找到匹配的路由表项
            for (const auto& [key, nodeId] : routingTable) {
                if (key.tableName == range.tableName && 
                    key.predicate.overlaps(range.range)) {
                    // 简单的投票机制
                    votes[nodeId] += range.isWrite ? 2 : 1;  // 写操作权重更大
                }
            }
        }
        
        // 返回得票最多的节点
        return max_element(votes.begin(), votes.end()) - votes.begin();
    }
}; 