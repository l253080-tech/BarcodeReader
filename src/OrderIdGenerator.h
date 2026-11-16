#pragma once
#include <string>
#include <unordered_set>

// Generates unique 8-digit order IDs: 6-digit sequence + 2-digit day of month
// (last two digits indicate the order date, as required).
// Call registerExistingId() for every ID already stored in Order.txt on startup.
class OrderIdGenerator {
public:
    void registerExistingId(const std::string& id);
    // dayOfMonth 1..31. Throws std::invalid_argument / std::runtime_error on bad day / exhausted IDs.
    std::string generate(int dayOfMonth);

private:
    std::unordered_set<std::string> used_;
};
