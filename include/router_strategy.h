#ifndef ROUTER_STRATEGY_H
#define ROUTER_STRATEGY_H

#include <vector>
#include <unordered_map>
#include <algorithm>
#include "range.h"
#include "hash_key.h"
#include "cost_calculator.h"

struct RoutingResult {
    int nodeId;
    int estimatedCost;
    
    RoutingResult(int node, int cost) : nodeId(node), estimatedCost(cost) {}
};

class RouterStrategy {
public:
    virtual ~RouterStrategy() = default;
    virtual RoutingResult route(
        const std::vector<RangeKey>& queryRanges,
        const std::unordered_map<HashKey, int>& routingTable,
        const std::vector<std::unordered_map<std::string, 
            std::unordered_map<std::string, std::vector<Range>>>>& nodeDataRanges,
        int nodeCount) = 0;
};

class BasicRouterStrategy : public RouterStrategy {
public:
    RoutingResult route(
        const std::vector<RangeKey>& queryRanges,
        const std::unordered_map<HashKey, int>& routingTable,
        const std::vector<std::unordered_map<std::string, 
            std::unordered_map<std::string, std::vector<Range>>>>& nodeDataRanges,
        int nodeCount) override;
};

class AdvancedRouterStrategy : public RouterStrategy {
private:
    struct NodeStats {
        double loadFactor;
        NodeStats() : loadFactor(0.0) {}
    };
    std::vector<NodeStats> nodeStats;

public:
    RoutingResult route(
        const std::vector<RangeKey>& queryRanges,
        const std::unordered_map<HashKey, int>& routingTable,
        const std::vector<std::unordered_map<std::string, 
            std::unordered_map<std::string, std::vector<Range>>>>& nodeDataRanges,
        int nodeCount) override;
};

#endif // ROUTER_STRATEGY_H 