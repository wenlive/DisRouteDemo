#include "cost_calculator.h"

bool CostCalculator::isDataInNode(
    const RangeKey& range, 
    int nodeId,
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges) {
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

int CostCalculator::calculateOperationCost(
    const RangeKey& range, 
    int nodeId,
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges) {
    bool isLocal = isDataInNode(range, nodeId, nodeDataRanges);
    
    if (range.isWrite) {
        return isLocal ? LOCAL_WRITE_COST : REMOTE_WRITE_COST;
    } else {
        return isLocal ? LOCAL_READ_COST : REMOTE_READ_COST;
    }
}

int CostCalculator::calculateTotalCost(
    const std::vector<RangeKey>& queryRanges, 
    int nodeId,
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges) {
    int totalCost = 0;
    for (const auto& range : queryRanges) {
        totalCost += calculateOperationCost(range, nodeId, nodeDataRanges);
    }
    return totalCost;
} 