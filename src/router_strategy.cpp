#include "router_strategy.h"
#include <algorithm>

RoutingResult BasicRouterStrategy::route(
    const std::vector<RangeKey>& queryRanges,
    const std::unordered_map<HashKey, int>& routingTable,
    const std::vector<std::unordered_map<std::string, 
        std::unordered_map<std::string, std::vector<Range>>>>& nodeDataRanges,
    int nodeCount) {
    
    std::vector<std::pair<int, int>> nodeCosts(nodeCount);
    
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
                            int overlap = std::min(range.upper, query.range.upper) - 
                                        std::max(range.lower, query.range.lower) + 1;
                            localDataSize += std::max(0, overlap);
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
    
    // 修改 lambda 表达式以适配 C++11
    auto bestNode = std::min_element(nodeCosts.begin(), nodeCosts.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second < b.second;
        });
        
    return RoutingResult(bestNode->first, bestNode->second);
}

RoutingResult AdvancedRouterStrategy::route(
    const std::vector<RangeKey>& queryRanges,
    const std::unordered_map<HashKey, int>& routingTable,
    const std::vector<std::unordered_map<std::string, 
        std::unordered_map<std::string, std::vector<Range>>>>& nodeDataRanges,
    int nodeCount) {
    
    // 确保 nodeStats 大小正确
    if (nodeStats.size() != static_cast<size_t>(nodeCount)) {
        nodeStats.resize(nodeCount);
    }
    
    // 首先使用基本策略计算代价
    std::vector<std::pair<int, int>> nodeCosts(nodeCount);
    
    for(int i = 0; i < nodeCount; i++) {
        nodeCosts[i].first = i;
        int totalCost = 0;
        
        for(const auto& query : queryRanges) {
            int localDataSize = 0;
            int remoteDataSize = 0;
            
            const auto& nodeData = nodeDataRanges[i];
            auto tableIt = nodeData.find(query.tableName);
            if (tableIt != nodeData.end()) {
                auto colIt = tableIt->second.find(query.columnName);
                if (colIt != tableIt->second.end()) {
                    for (const auto& range : colIt->second) {
                        if (range.overlaps(query.range)) {
                            int overlap = std::min(range.upper, query.range.upper) - 
                                        std::max(range.lower, query.range.lower) + 1;
                            localDataSize += std::max(0, overlap);
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
        
        // 考虑负载因子
        double loadAdjustedCost = totalCost * (1.0 + nodeStats[i].loadFactor);
        nodeCosts[i].second = static_cast<int>(loadAdjustedCost);
    }
    
    // 修改 lambda 表达式以适配 C++11
    auto bestNode = std::min_element(nodeCosts.begin(), nodeCosts.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second < b.second;
        });
    
    // 更新选中节点的负载因子
    nodeStats[bestNode->first].loadFactor += 0.1;  // 可调整增长率
    
    return RoutingResult(bestNode->first, bestNode->second);
}