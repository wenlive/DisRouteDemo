#ifndef COST_CALCULATOR_H
#define COST_CALCULATOR_H

#include <vector>
#include <unordered_map>
#include <string>
#include "range.h"

class CostCalculator {
public:
    static constexpr int LOCAL_READ_COST = 1;
    static constexpr int REMOTE_READ_COST = 10;
    static constexpr int LOCAL_WRITE_COST = 2;
    static constexpr int REMOTE_WRITE_COST = 20;

    static int calculateTotalCost(
        const std::vector<RangeKey>& queryRanges,
        int nodeId,
        const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges);

private:
    static int calculateOperationCost(
        const RangeKey& range,
        int nodeId,
        const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges);

    static bool isDataInNode(
        const RangeKey& range,
        int nodeId,
        const std::vector<std::unordered_map<std::string, std::vector<Range>>>& nodeDataRanges);
};

#endif // COST_CALCULATOR_H 