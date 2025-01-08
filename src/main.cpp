#include <iostream>
#include <vector>
#include "router.h"

void runTest(QueryRouter& router, const std::vector<std::vector<RangeKey>>& testQueries) {
    for (const auto& query : testQueries) {
        int nodeId = router.route(query);
        std::cout << "Query routed to node: " << nodeId << std::endl;
    }
}

int main() {
    const int NODE_COUNT = 4;
    
    // 创建路由器实例
    QueryRouter basicRouter(NODE_COUNT, RoutingMode::BASIC);
    QueryRouter advancedRouter(NODE_COUNT, RoutingMode::ADVANCED);
    
    // 初始化数据分布
    std::vector<std::pair<std::string, std::vector<Range>>> distribution = {
        {"orders", {Range(1, 1000), Range(1001, 2000)}},
        {"customers", {Range(1, 500), Range(501, 1000)}}
    };
    
    basicRouter.initializeDataDistribution(distribution);
    advancedRouter.initializeDataDistribution(distribution);
    
    // 准备测试查询
    std::vector<std::vector<RangeKey>> testQueries = {
        {
            RangeKey("orders", "id", Range(1, 100), false)
        },
        {
            RangeKey("orders", "id", Range(1, 100), false),
            RangeKey("customers", "id", Range(1, 50), true)
        }
    };
    
    std::cout << "Testing Basic Router:" << std::endl;
    runTest(basicRouter, testQueries);
    
    std::cout << "\nTesting Advanced Router:" << std::endl;
    runTest(advancedRouter, testQueries);
    
    return 0;
} 