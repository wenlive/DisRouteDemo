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
             int nodeCount) override;
};

class AdvancedRouterStrategy : public RouterStrategy {
private:
    struct NodeStats {
        int dataCount;
        double loadFactor;
        NodeStats() : dataCount(0), loadFactor(0.0) {}
    };
    std::vector<NodeStats> nodeStats;

    double calculateMatchScore(const RangeKey& range, int nodeId,
                             const std::unordered_map<HashKey, int>& routingTable);

public:
    int route(const std::vector<RangeKey>& queryRanges,
             const std::unordered_map<HashKey, int>& routingTable,
             const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges,
             int nodeCount) override;
};

#endif // ROUTER_STRATEGY_H 