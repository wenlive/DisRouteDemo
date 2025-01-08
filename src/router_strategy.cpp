#include "router_strategy.h"

RoutingResult BasicRouterStrategy::route(
    const std::vector<RangeKey>& queryRanges,
    const std::unordered_map<HashKey, int>& routingTable,
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges,
    int nodeCount) {
    
    vector<pair<int, int>> nodeCosts(nodeCount); // pair<nodeId, totalCost>
    
    for(int i = 0; i < nodeCount; i++) {
        nodeCosts[i].first = i;
        int totalCost = 0;
        
        // 对每个查询范围计算数据重叠量
        for(const auto& query : queryRanges) {
            int localDataSize = 0;
            int remoteDataSize = 0;
            
            // 计算与当前节点数据的重叠量
            const auto& nodeRanges = nodeDataRanges[i];
            auto tableIt = nodeRanges.find(query.tableName);
            if (tableIt != nodeRanges.end()) {
                for (const auto& range : tableIt->second) {
                    if (range.overlaps(query.range)) {
                        // 计算重叠的数据量
                        int overlap = min(range.upper, query.range.upper) - 
                                    max(range.lower, query.range.lower) + 1;
                        localDataSize += max(0, overlap);
                    }
                }
            }
            
            // 计算需要从其他节点获取的数据量
            int querySize = query.range.upper - query.range.lower + 1;
            remoteDataSize = querySize - localDataSize;
            
            // 写操作的特殊处理：考虑数据迁移后的影响
            if (query.isWrite) {
                // 写操作后，这部分数据将在本地可用，降低后续操作的代价
                totalCost += remoteDataSize * REMOTE_WRITE_COST;  // 初始写入代价
                // 不计入本地数据的写代价，因为这是不可避免的
            } else {
                totalCost += localDataSize * LOCAL_READ_COST +
                            remoteDataSize * REMOTE_READ_COST;
            }
        }
        
        nodeCosts[i].second = totalCost;
    }
    
    // 选择代价最小的节点
    auto bestNode = min_element(nodeCosts.begin(), nodeCosts.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
        
    return RoutingResult(bestNode->first, bestNode->second);
}

double AdvancedRouterStrategy::calculateMatchScore(
    const RangeKey& range, 
    int nodeId,
    const std::unordered_map<HashKey, int>& routingTable) {
    double score = 0.0;
    for (const auto& pair : routingTable) {
        if (pair.second == nodeId && pair.first.tableName == range.tableName) {
            if (pair.first.predicate.overlaps(range.range)) {
                score += range.isWrite ? 2.0 : 1.0;
            }
        }
    }
    return score;
}

int AdvancedRouterStrategy::route(
    const std::vector<RangeKey>& queryRanges,
    const std::unordered_map<HashKey, int>& routingTable,
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges,
    int nodeCount) {
    nodeStats.resize(nodeCount);
    std::vector<double> weights(nodeCount, 0.0);
    
    for (int nodeId = 0; nodeId < nodeCount; nodeId++) {
        for (const auto& range : queryRanges) {
            double matchScore = calculateMatchScore(range, nodeId, routingTable);
            weights[nodeId] += matchScore * (1.0 / (1.0 + nodeStats[nodeId].loadFactor));
        }
    }
    
    int bestNode = std::max_element(weights.begin(), weights.end()) - weights.begin();
    nodeStats[bestNode].loadFactor += 0.1;
    
    return bestNode;
} 