class CostCalculator {
private:
    static constexpr int LOCAL_READ_COST = 1;
    static constexpr int REMOTE_READ_COST = 10;
    static constexpr int LOCAL_WRITE_COST = 2;
    static constexpr int REMOTE_WRITE_COST = 20;

public:
    // 检查数据范围是否在指定节点上
    static bool isDataInNode(const RangeKey& range, int nodeId, 
                           const vector<unordered_map<string, vector<Range>>>& nodeDataRanges) {
        const auto& nodeRanges = nodeDataRanges[nodeId];
        auto it = nodeRanges.find(range.tableName);
        if (it == nodeRanges.end()) return false;
        
        for (const auto& nodeRange : it->second) {
            if (nodeRange.overlaps(range.range)) {
                return true;
            }
        }
        return false;
    }

    static int calculateOperationCost(const RangeKey& range, int nodeId,
                                    const vector<unordered_map<string, vector<Range>>>& nodeDataRanges) {
        bool isLocal = isDataInNode(range, nodeId, nodeDataRanges);
        
        if (range.isWrite) {
            return isLocal ? LOCAL_WRITE_COST : REMOTE_WRITE_COST;
        } else {
            return isLocal ? LOCAL_READ_COST : REMOTE_READ_COST;
        }
    }

    static int calculateTotalCost(const vector<RangeKey>& queryRanges, int nodeId,
                                const vector<unordered_map<string, vector<Range>>>& nodeDataRanges) {
        int totalCost = 0;
        for (const auto& range : queryRanges) {
            totalCost += calculateOperationCost(range, nodeId, nodeDataRanges);
        }
        return totalCost;
    }
}; 