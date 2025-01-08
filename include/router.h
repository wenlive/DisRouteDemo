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

struct TableDistribution {
    std::string tableName;
    std::unordered_map<std::string, std::vector<Range>> columnRanges;  // column -> ranges
};

class QueryRouter {
private:
    std::unordered_map<HashKey, int> routingTable;
    const int nodeCount;
    std::vector<std::unordered_map<std::string, 
        std::unordered_map<std::string, std::vector<Range>>>> nodeDataRanges;
    std::unique_ptr<RouterStrategy> strategy;
    RoutingMode currentMode;

    // 处理写操作导致的数据迁移
    void handleWriteOperation(const RangeKey& writeOp, int targetNodeId) {
        if (!writeOp.isWrite) return;
        
        // 更新目标节点的数据范围
        nodeDataRanges[targetNodeId][writeOp.tableName][writeOp.columnName]
            .push_back(writeOp.range);
        
        // 更新路由表
        std::vector<ColumnRange> ranges = {ColumnRange(writeOp.columnName, writeOp.range)};
        HashKey key(writeOp.tableName, ranges);
        routingTable[key] = targetNodeId;
    }

public:
    QueryRouter(int nodes, RoutingMode mode = RoutingMode::BASIC);
    void setRoutingMode(RoutingMode mode);
    RoutingMode getRoutingMode() const;
    RoutingResult route(const std::vector<RangeKey>& queryRanges);
    void initializeDataDistribution(const std::vector<TableDistribution>& distribution);
    const std::vector<std::unordered_map<std::string, 
        std::unordered_map<std::string, std::vector<Range>>>>& getNodeDataRanges() const {
        return nodeDataRanges;
    }
};

#endif // ROUTER_H 