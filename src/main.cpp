#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <set>
#include <algorithm>
#include "range.h"
#include "hash_key.h"
#include "router.h"
#include "cost_calculator.h"

// 生成随机范围的辅助函数
Range generateRandomRange(int minVal, int maxVal, int maxRangeSize, std::mt19937& gen) {
    std::uniform_int_distribution<> startDis(minVal, maxVal - 1);
    int start = startDis(gen);
    std::uniform_int_distribution<> sizeDis(1, std::min(maxRangeSize, maxVal - start));
    int size = sizeDis(gen);
    return Range(start, start + size - 1);
}

// 生成大量测试查询
std::vector<std::vector<RangeKey>> generateTestQueries(int queryCount) {
    // 使用当前时间作为随机数种子
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    
    // 配置参数
    const double WRITE_PROBABILITY = 0.1;  // 10%的写操作概率
    const double MULTI_TABLE_PROBABILITY = 0.3;  // 30%的多表查询概率
    
    std::vector<std::vector<RangeKey>> queries;
    std::uniform_real_distribution<> probDis(0.0, 1.0);
    
    for (int i = 0; i < queryCount; ++i) {
        std::vector<RangeKey> query;
        
        // 决定是否是多表查询
        bool isMultiTable = (probDis(gen) < MULTI_TABLE_PROBABILITY);
        
        // orders表查询
        Range orderRange = generateRandomRange(1, 2000, 200, gen);
        bool isWrite = (probDis(gen) < WRITE_PROBABILITY);
        query.emplace_back("orders", "id", orderRange, isWrite);
        
        // 可能的customers表查询
        if (isMultiTable) {
            Range customerRange = generateRandomRange(1, 1000, 100, gen);
            bool isWrite = (probDis(gen) < WRITE_PROBABILITY);
            query.emplace_back("customers", "id", customerRange, isWrite);
        }
        
        queries.push_back(query);
    }
    
    return queries;
}

// 添加数据分布分析函数
void printDataDistribution(
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges, 
    const std::string& title) {
    std::cout << "\n=== " << title << " ===" << std::endl;
    
    for (size_t nodeId = 0; nodeId < nodeDataRanges.size(); ++nodeId) {
        std::cout << "\nNode " << nodeId << ":" << std::endl;
        const auto& nodeData = nodeDataRanges[nodeId];
        
        // 替换结构化绑定
        for (const auto& tableEntry : nodeData) {
            const std::string& tableName = tableEntry.first;
            const std::vector<Range>& ranges = tableEntry.second;
            
            std::cout << "  Table '" << tableName << "':" << std::endl;
            for (const auto& range : ranges) {
                std::cout << "    Range [" << range.lower << "-" << range.upper << "]"
                         << " (size: " << (range.upper - range.lower + 1) << ")" << std::endl;
            }
        }
    }
}

void analyzeDataDistributionChanges(
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& initial,
    const std::vector<std::unordered_map<std::string, std::vector<Range>>>& current) {
    
    std::cout << "\n=== Data Distribution Changes Analysis ===" << std::endl;
    
    for (size_t nodeId = 0; nodeId < initial.size(); ++nodeId) {
        std::cout << "\nNode " << nodeId << " changes:" << std::endl;
        const auto& initialNode = initial[nodeId];
        const auto& currentNode = current[nodeId];
        
        // 收集所有表名
        std::set<std::string> allTables;
        for (const auto& tableEntry : initialNode) {
            allTables.insert(tableEntry.first);
        }
        for (const auto& tableEntry : currentNode) {
            allTables.insert(tableEntry.first);
        }
        
        // 分析每个表的变化
        for (const auto& tableName : allTables) {
            std::cout << "  Table '" << tableName << "':" << std::endl;
            
            // 获取初始和当前的范围
            const auto initialIt = initialNode.find(tableName);
            const auto currentIt = currentNode.find(tableName);
            
            const std::vector<Range>& initialRanges = 
                (initialIt != initialNode.end()) ? initialIt->second : std::vector<Range>();
            const std::vector<Range>& currentRanges = 
                (currentIt != currentNode.end()) ? currentIt->second : std::vector<Range>();
            
            // 计算数据量变化
            int initialTotal = 0;
            for (const auto& range : initialRanges) {
                initialTotal += range.upper - range.lower + 1;
            }
            
            int currentTotal = 0;
            for (const auto& range : currentRanges) {
                currentTotal += range.upper - range.lower + 1;
            }
            
            int change = currentTotal - initialTotal;
            std::cout << "    Initial data size: " << initialTotal << std::endl;
            std::cout << "    Current data size: " << currentTotal << std::endl;
            std::cout << "    Change: " << (change >= 0 ? "+" : "") << change 
                      << " (" << (change * 100.0 / (initialTotal ? initialTotal : 1)) 
                      << "%)" << std::endl;
        }
    }
}

void runBasicTest(QueryRouter& router, const std::vector<std::vector<RangeKey>>& testQueries) {
    // 保存初始数据分布
    auto initialDistribution = router.getNodeDataRanges();
    
    // 打印初始数据分布
    printDataDistribution(initialDistribution, "Initial Data Distribution");
    
    // 统计信息
    std::vector<int> nodeSelectionCount(4, 0);
    int totalCost = 0;
    int writeOps = 0;
    int crossNodeQueries = 0;
    
    for (size_t i = 0; i < testQueries.size(); ++i) {
        const auto& query = testQueries[i];
        
        bool printDetail = (i % 10 == 0);
        if (printDetail) {
            std::cout << "\nQuery " << i + 1 << ":" << std::endl;
            for (const auto& range : query) {
                std::cout << "Table: " << range.tableName 
                          << ", Range: [" << range.range.lower << "-" << range.range.upper << "]"
                          << ", IsWrite: " << (range.isWrite ? "true" : "false") << std::endl;
            }
        }
        
        RoutingResult result = router.route(query);
        
        nodeSelectionCount[result.nodeId]++;
        totalCost += result.estimatedCost;
        
        bool hasWrite = false;
        for (const auto& range : query) {
            if (range.isWrite) {
                hasWrite = true;
                break;
            }
        }
        if (hasWrite) writeOps++;
        
        if (result.estimatedCost > (static_cast<int>(query.size()) * CostCalculator::LOCAL_READ_COST)) {
            crossNodeQueries++;
        }
        
        if (printDetail) {
            std::cout << "Routed to node: " << result.nodeId 
                      << ", Estimated cost: " << result.estimatedCost << std::endl;
        }
    }
    
    // 打印统计信息
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Total queries: " << testQueries.size() << std::endl;
    std::cout << "Write operations: " << writeOps 
              << " (" << (writeOps * 100.0 / testQueries.size()) << "%)" << std::endl;
    std::cout << "Cross-node queries: " << crossNodeQueries 
              << " (" << (crossNodeQueries * 100.0 / testQueries.size()) << "%)" << std::endl;
    std::cout << "Average cost per query: " << (totalCost * 1.0 / testQueries.size()) << std::endl;
    std::cout << "\nNode selection distribution:" << std::endl;
    for (size_t i = 0; i < nodeSelectionCount.size(); ++i) {
        std::cout << "Node " << i << ": " << nodeSelectionCount[i] 
                  << " (" << (nodeSelectionCount[i] * 100.0 / testQueries.size()) << "%)" << std::endl;
    }
    
    // 打印当前数据分布
    auto currentDistribution = router.getNodeDataRanges();
    printDataDistribution(currentDistribution, "Current Data Distribution");
    
    // 分析变化
    analyzeDataDistributionChanges(initialDistribution, currentDistribution);
}

int main() {
    const int NODE_COUNT = 4;
    QueryRouter basicRouter(NODE_COUNT, RoutingMode::BASIC);
    
    // 初始化数据分布
    std::vector<TableDistribution> distribution = {
        {
            "orders", 
            {
                {"id", {Range(1, 1000), Range(1001, 2000)}},
                {"user_id", {Range(1, 500), Range(501, 1000)}}
            }
        },
        {
            "customers",
            {
                {"id", {Range(1, 500), Range(501, 1000)}}
            }
        }
    };
    
    basicRouter.initializeDataDistribution(distribution);
    
    // 生成1000个测试查询
    auto testQueries = generateTestQueries(1000);
    
    // 运行测试并收集统计信息
    runBasicTest(basicRouter, testQueries);
    
    return 0;
} 