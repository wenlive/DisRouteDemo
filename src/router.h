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
    QueryRouter(int nodes, RoutingMode mode = RoutingMode::BASIC) 
        : nodeCount(nodes), currentMode(mode), nodeDataRanges(nodes) {
        setRoutingMode(mode);
    }

    void setRoutingMode(RoutingMode mode) {
        currentMode = mode;
        switch (mode) {
            case RoutingMode::BASIC:
                strategy.reset(new BasicRouterStrategy());
                break;
            case RoutingMode::ADVANCED:
                strategy.reset(new AdvancedRouterStrategy());
                break;
        }
    }

    RoutingMode getRoutingMode() const {
        return currentMode;
    }

    int route(const std::vector<RangeKey>& queryRanges) {
        return strategy->route(queryRanges, routingTable, nodeDataRanges, nodeCount);
    }

    void initializeDataDistribution(const std::vector<std::pair<std::string, std::vector<Range>>>& distribution) {
        for (int nodeId = 0; nodeId < nodeCount; nodeId++) {
            for (const auto& tableDist : distribution) {
                const std::string& tableName = tableDist.first;
                for (const Range& range : tableDist.second) {
                    HashKey key(tableName, "id", range);
                    routingTable[key] = nodeId;
                    nodeDataRanges[nodeId][tableName].push_back(range);
                }
            }
        }
    }

    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& getNodeDataRanges() const {
        return nodeDataRanges;
    }
};

#endif // ROUTER_H 