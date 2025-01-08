RoutingResult BasicRouterStrategy::route(
    const std::vector<RangeKey>& queryRanges,
    const std::unordered_map<HashKey, int>& routingTable,
    const std::vector<std::unordered_map<std::string, 
        std::unordered_map<std::string, std::vector<Range>>>>& nodeDataRanges,
    int nodeCount) {
    
    vector<pair<int, int>> nodeCosts(nodeCount);
    
    for(int i = 0; i < nodeCount; i++) {
        nodeCosts[i].first = i;
        int totalCost = 0;
        
        for(const auto& query : queryRanges) {
            int localDataSize = 0;
            int remoteDataSize = 0;
            
            // 计算与当前节点数据的重叠量
            const auto& nodeData = nodeDataRanges[i];
            auto tableIt = nodeData.find(query.tableName);
            if (tableIt != nodeData.end()) {
                auto colIt = tableIt->second.find(query.columnName);
                if (colIt != tableIt->second.end()) {
                    for (const auto& range : colIt->second) {
                        if (range.overlaps(query.range)) {
                            int overlap = min(range.upper, query.range.upper) - 
                                        max(range.lower, query.range.lower) + 1;
                            localDataSize += max(0, overlap);
                        }
                    }
                }
            }
            
            int querySize = query.range.upper - query.range.lower + 1;
            remoteDataSize = querySize - localDataSize;
            
            if (query.isWrite) {
                totalCost += remoteDataSize * CostCalculator::REMOTE_WRITE_COST;
            } else {
                totalCost += localDataSize * CostCalculator::LOCAL_READ_COST +
                            remoteDataSize * CostCalculator::REMOTE_READ_COST;
            }
        }
        
        nodeCosts[i].second = totalCost;
    }
    
    auto bestNode = min_element(nodeCosts.begin(), nodeCosts.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
        
    return RoutingResult(bestNode->first, bestNode->second);
}