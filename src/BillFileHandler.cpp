#include "BillFileHandler.h"
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

using namespace std;

BillFileHandler::BillFileHandler(string billFilePath)
    : billFilePath_(move(billFilePath)) {
}

string BillFileHandler::encodeToken(const string& raw) {
    if (raw.empty()) {
        return "-";
    }
    string out = raw;
    for (char& c : out) {
        if (c == ' ') {
            c = '_';
        }
    }
    return out;
}

string BillFileHandler::decodeToken(const string& token) {
    if (token == "-") {
        return "";
    }
    string out = token;
    for (char& c : out) {
        if (c == '_') {
            c = ' ';
        }
    }
    return out;
}

bool BillFileHandler::validateBillRecord(const Bill& bill) const {
    if (bill.getBillId().size() != 9) {
        return false;
    }
    for (char c : bill.getBillId()) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    if (bill.getOrderId().size() != 8) {
        return false;
    }
    if (bill.getItems().empty()) {
        return false;
    }
    if (bill.getPaymentStatus().empty()) {
        return false;
    }

    for (const auto& item : bill.getItems()) {
        if (item.productAcronym.empty()) {
            return false;
        }
        if (item.unitPriceWithoutTax < 0.0 ||
            item.taxPerUnit < 0.0 ||
            item.discountPerUnit < 0.0 ||
            item.quantity <= 0.0) {
            return false;
        }
    }

    double expectedPayable = bill.getTotalWithoutTax() + bill.getTotalTax() - bill.getTotalDiscount();
    if (expectedPayable < 0.0) {
        expectedPayable = 0.0;
    }
    if (fabs(expectedPayable - bill.getTotalPayable()) > 1e-4) {
        return false;
    }

    return true;
}

string BillFileHandler::formatBillLine(const Bill& bill) const {
    ostringstream itemsStream;
    itemsStream << setprecision(10);
    const auto& items = bill.getItems();
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) {
            itemsStream << ";";
        }
        itemsStream << encodeToken(items[i].productAcronym) << ":"
                    << items[i].unitPriceWithoutTax << ":"
                    << items[i].taxPerUnit << ":"
                    << items[i].discountPerUnit << ":"
                    << items[i].quantity;
    }

    ostringstream out;
    out << setprecision(10)
        << bill.getBillId() << " "
        << encodeToken(bill.getCustomerName()) << " "
        << bill.getOrderId() << " "
        << itemsStream.str() << " "
        << bill.getTotalWithoutTax() << " "
        << bill.getTotalTax() << " "
        << bill.getTotalDiscount() << " "
        << bill.getTotalPayable() << " "
        << encodeToken(bill.getPaymentStatus()) << " #";
    return out.str();
}

bool BillFileHandler::parseBillLine(const string& rawLine, Bill& out) const {
    string line = rawLine;
    while (!line.empty() && isspace(static_cast<unsigned char>(line.back()))) {
        line.pop_back();
    }
    if (line.empty() || line.back() != '#') {
        return false;
    }
    line.pop_back();

    istringstream stream(line);
    vector<string> t;
    string word;
    while (stream >> word) {
        t.push_back(word);
    }

    if (t.size() != 9) {
        return false;
    }

    vector<BillItem> items;
    istringstream itemListStream(t[3]);
    string itemEntry;
    while (getline(itemListStream, itemEntry, ';')) {
        if (itemEntry.empty()) {
            continue;
        }
        istringstream partStream(itemEntry);
        vector<string> parts;
        string part;
        while (getline(partStream, part, ':')) {
            parts.push_back(part);
        }
        if (parts.size() != 5) {
            return false;
        }

        try {
            BillItem bi;
            bi.productAcronym      = decodeToken(parts[0]);
            bi.unitPriceWithoutTax = stod(parts[1]);
            bi.taxPerUnit          = stod(parts[2]);
            bi.discountPerUnit     = stod(parts[3]);
            bi.quantity            = stod(parts[4]);
            items.push_back(move(bi));
        } catch (...) {
            return false;
        }
    }

    Bill parsed(t[0], decodeToken(t[1]), t[2], move(items), decodeToken(t[8]));
    if (!validateBillRecord(parsed)) {
        return false;
    }

    out = move(parsed);
    return true;
}

bool BillFileHandler::writeLinesAtomic(const string& targetPath, const vector<string>& lines) const {
    string tempPath = targetPath + ".tmp";
    {
        ofstream out(tempPath, ios::trunc);
        if (!out.is_open()) {
            return false;
        }
        for (const auto& l : lines) {
            out << l << "\n";
        }
        out.flush();
        const bool flushed = out.good();
        out.close();
        if (!flushed || out.fail()) {
            return false;
        }
    }

    error_code ec;
    filesystem::rename(tempPath, targetPath, ec);
    if (ec) {
        filesystem::remove(tempPath, ec);
        return false;
    }
    return true;
}

bool BillFileHandler::saveBill(const Bill& bill) const {
    if (!validateBillRecord(bill)) {
        return false;
    }

    vector<string> lines;
    ifstream in(billFilePath_);
    if (in.is_open()) {
        string line;
        while (getline(in, line)) {
            Bill existing;
            if (parseBillLine(line, existing) && existing.getBillId() == bill.getBillId()) {
                return false;
            }
            lines.push_back(line);
        }
        in.close();
    }

    lines.push_back(formatBillLine(bill));
    return writeLinesAtomic(billFilePath_, lines);
}

vector<Bill> BillFileHandler::loadBills() const {
    vector<Bill> bills;
    ifstream in(billFilePath_);
    if (!in.is_open()) {
        return bills;
    }

    string line;
    while (getline(in, line)) {
        Bill b;
        if (parseBillLine(line, b)) {
            bills.push_back(move(b));
        }
    }
    return bills;
}

optional<Bill> BillFileHandler::findBillById(const string& billId) const {
    ifstream in(billFilePath_);
    if (!in.is_open()) {
        return nullopt;
    }

    string line;
    while (getline(in, line)) {
        Bill b;
        if (parseBillLine(line, b) && b.getBillId() == billId) {
            return b;
        }
    }
    return nullopt;
}
