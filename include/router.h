#ifndef ROUTER_H
#define ROUTER_H

#include <memory>
#include <vector>
#include <unordered_map>
#include "router_strategy.h"

enum class RoutingMode {
    BASIC,
    ADVANCED
};

class QueryRouter {
private:
    std::unordered_map<HashKey, int> routingTable;
    const int nodeCount;
    std::vector<std::unordered_map<std::string, std::vector<Range>>> nodeDataRanges;
    std::unique_ptr<RouterStrategy> strategy;
    RoutingMode currentMode;

public:
    QueryRouter(int nodes, RoutingMode mode = RoutingMode::BASIC);
    void setRoutingMode(RoutingMode mode);
    RoutingMode getRoutingMode() const;
    int route(const std::vector<RangeKey>& queryRanges);
    void initializeDataDistribution(const std::vector<std::pair<std::string, std::vector<Range>>>& distribution);
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& getNodeDataRanges() const;
};

#endif // ROUTER_H 