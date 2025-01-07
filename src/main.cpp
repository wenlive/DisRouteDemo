#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <ctime>
#include "query_router.h"
#include "cost_calculator.h"

using namespace std;

void runTest(QueryRouter& router, const vector<vector<RangeKey>>& testQueries,
            const vector<unordered_map<string, vector<Range>>>& nodeDataRanges) {
    int totalRoutedCost = 0;
    int totalRandomCost = 0;
    const int TEST_ITERATIONS = 100;
    const int NODE_COUNT = 4;
    
    for (const auto& query : testQueries) {
        // 使用路由策略
        int routedNode = router.route(query);
        int routedCost = CostCalculator::calculateTotalCost(query, routedNode, nodeDataRanges);
        totalRoutedCost += routedCost;
        
        // 随机策略
        int randomCostSum = 0;
        for (int i = 0; i < TEST_ITERATIONS; i++) {
            int randomNode = rand() % NODE_COUNT;
            randomCostSum += CostCalculator::calculateTotalCost(query, randomNode, nodeDataRanges);
        }
        totalRandomCost += randomCostSum / TEST_ITERATIONS;
    }
    
    cout << "Average routed cost per query: " << (double)totalRoutedCost / testQueries.size() << endl;
    cout << "Average random cost per query: " << (double)totalRandomCost / testQueries.size() << endl;
    cout << "Cost reduction: " 
         << (1.0 - (double)totalRoutedCost / totalRandomCost) * 100 << "%" << endl;
}

int main() {
    const int NODE_COUNT = 4;
    QueryRouter router(NODE_COUNT);
    
    // 初始化数据分布
    vector<pair<string, vector<Range>>> distribution = {
        {"orders", {
            {1, 1000},    // Node 0
            {1001, 2000}, // Node 1
            {2001, 3000}, // Node 2
            {3001, 4000}  // Node 3
        }},
        {"customers", {
            {1, 500},     // Node 0
            {501, 1000},  // Node 1
            {1001, 1500}, // Node 2
            {1501, 2000}  // Node 3
        }}
    };
    
    router.initializeDataDistribution(distribution);
    
    // 准备测试查询
    vector<vector<RangeKey>> testQueries = {
        // 单表查询
        {
            RangeKey("orders", "id", {1, 100}, false)
        },
        // 多表查询
        {
            RangeKey("orders", "id", {1, 100}, false),
            RangeKey("customers", "id", {1, 50}, true)
        },
        // 跨节点查询
        {
            RangeKey("orders", "id", {900, 1100}, false)
        }
    };
    
    runTest(router, testQueries, router.getNodeDataRanges());
    
    return 0;
} 