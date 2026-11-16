#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Bill.h"

using namespace std;

class BillFileHandler {
public:
    explicit BillFileHandler(string billFilePath = "Bill.txt");

    bool saveBill(const Bill& bill) const;
    vector<Bill> loadBills() const;
    optional<Bill> findBillById(const string& billId) const;
    bool validateBillRecord(const Bill& bill) const;

private:
    static string encodeToken(const string& raw);
    static string decodeToken(const string& token);

    string formatBillLine(const Bill& bill) const;
    bool parseBillLine(const string& rawLine, Bill& out) const;
    bool writeLinesAtomic(const string& targetPath, const vector<string>& lines) const;

    string billFilePath_;
};