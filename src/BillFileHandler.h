#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Bill.h"


class BillFileHandler {
public:
    explicit BillFileHandler(std::string billFilePath = "Bill.txt");

    bool saveBill(const Bill& bill) const;
    std::vector<Bill> loadBills() const;
    std::optional<Bill> findBillById(const std::string& billId) const;
    bool validateBillRecord(const Bill& bill) const;
    const std::string& billFilePath() const { return billFilePath_; }

private:
    static std::string encodeToken(const std::string& raw);
    static std::string decodeToken(const std::string& token);

    std::string formatBillLine(const Bill& bill) const;
    bool parseBillLine(const std::string& rawLine, Bill& out) const;
    bool writeLinesAtomic(const std::string& targetPath, const std::vector<std::string>& lines) const;

    std::string billFilePath_;
};