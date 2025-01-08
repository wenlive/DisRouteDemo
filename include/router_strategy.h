#ifndef ROUTER_STRATEGY_H
#define ROUTER_STRATEGY_H

#include <vector>
#include <unordered_map>
#include <algorithm>
#include "range.h"
#include "hash_key.h"

class RouterStrategy {
public:
    virtual ~RouterStrategy() = default;
    virtual int route(const std::vector<RangeKey>& queryRanges,
                     const std::unordered_map<HashKey, int>& routingTable,
                     const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges,
                     int nodeCount) = 0;
};

class BasicRouterStrategy : public RouterStrategy {
public:
    int route(const std::vector<RangeKey>& queryRanges,
             const std::unordered_map<HashKey, int>& routingTable,
             const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges,
             int nodeCount) override {
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
};

class AdvancedRouterStrategy : public RouterStrategy {
private:
    struct NodeStats {
        int dataCount;
        double loadFactor;
        NodeStats() : dataCount(0), loadFactor(0.0) {}
    };
    std::vector<NodeStats> nodeStats;

public:
    int route(const std::vector<RangeKey>& queryRanges,
             const std::unordered_map<HashKey, int>& routingTable,
             const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges,
             int nodeCount) override {
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

private:
    double calculateMatchScore(const RangeKey& range, int nodeId,
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
};

#endif // ROUTER_STRATEGY_H 