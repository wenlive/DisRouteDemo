#include "router_strategy.h"

int BasicRouterStrategy::route(
    const std::vector<RangeKey>& queryRanges,
    const std::unordered_map<HashKey, int>& routingTable,
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges,
    int nodeCount) {
    std::vector<int> votes(nodeCount, 0);
    
    for (const auto& range : queryRanges) {
        for (const auto& pair : routingTable) {
            if (pair.first.tableName == range.tableName && 
                pair.first.predicate.overlaps(range.range)) {
                votes[pair.second] += range.isWrite ? 2 : 1;
            }
        }
    }
    
    return std::max_element(votes.begin(), votes.end()) - votes.begin();
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