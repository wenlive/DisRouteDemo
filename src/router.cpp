#include "router.h"

QueryRouter::QueryRouter(int nodes, RoutingMode mode) 
    : nodeCount(nodes), currentMode(mode), nodeDataRanges(nodes) {
    setRoutingMode(mode);
}

void QueryRouter::setRoutingMode(RoutingMode mode) {
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

RoutingMode QueryRouter::getRoutingMode() const {
    return currentMode;
}

int QueryRouter::route(const std::vector<RangeKey>& queryRanges) {
    return strategy->route(queryRanges, routingTable, nodeDataRanges, nodeCount);
}

void QueryRouter::initializeDataDistribution(
    const std::vector<std::pair<std::string, std::vector<Range>>>& distribution) {
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

const std::vector<std::unordered_map<std::string, std::vector<Range>>>& 
QueryRouter::getNodeDataRanges() const {
    return nodeDataRanges;
} 