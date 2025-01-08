#include "router.h"

QueryRouter::QueryRouter(int nodes, RoutingMode mode)
    : nodeCount(nodes)
    , nodeDataRanges(nodes)
    , currentMode(mode) {
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

RoutingResult QueryRouter::route(const std::vector<RangeKey>& queryRanges) {
    RoutingResult result = strategy->route(queryRanges, routingTable, nodeDataRanges, nodeCount);
    
    // 处理写操作导致的数据迁移
    for (const auto& range : queryRanges) {
        if (range.isWrite) {
            handleWriteOperation(range, result.nodeId);
        }
    }
    
    return result;
}

void QueryRouter::initializeDataDistribution(const std::vector<TableDistribution>& distribution) {
    for (int nodeId = 0; nodeId < nodeCount; nodeId++) {
        for (const auto& tableDist : distribution) {
            const std::string& tableName = tableDist.tableName;
            
            // 为每个列的范围组合创建路由表项
            for (const auto& colRanges : tableDist.columnRanges) {
                const std::string& colName = colRanges.first;
                for (const Range& range : colRanges.second) {
                    // 更新路由表
                    std::vector<ColumnRange> columnRanges = {ColumnRange(colName, range)};
                    HashKey key(tableName, columnRanges);
                    routingTable[key] = nodeId;
                    
                    // 更新节点数据范围
                    nodeDataRanges[nodeId][tableName][colName].push_back(range);
                }
            }
        }
    }
}