#include "OrderFileHandler.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

using namespace std;

OrderFileHandler::OrderFileHandler(string orderFilePath, string orderDetailFilePath)
    : orderFilePath_(move(orderFilePath)),
    orderDetailFilePath_(move(orderDetailFilePath)) {
}

string OrderFileHandler::encodeToken(const string& raw) {
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

string OrderFileHandler::decodeToken(const string& token) {
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

string OrderFileHandler::formatOrderLine(const Order& order) const {
    ostringstream out;
    out << setprecision(10)
        << order.getOrderId() << " "
        << order.getOrderDate() << " "
        << order.getOrderTime() << " "
        << encodeToken(order.getCustomerName()) << " "
        << encodeToken(order.getCustomerContact()) << " "
        << toString(order.getOrderStatus()) << " "
        << order.getPaymentAmount() << " "
        << toString(order.getPaymentMode()) << " "
        << encodeToken(order.getMerchantName()) << " #";
    return out.str();
}

string OrderFileHandler::formatDetailLine(const string& orderId, const OrderItem& item) const {
    ostringstream out;
    out << setprecision(10)
        << orderId << " "
        << item.getProductId() << " "
        << item.getBarcode() << " "
        << item.getQuantity() << " #";
    return out.str();
}

bool OrderFileHandler::parseOrderLine(const string& rawLine, Order& out) const {
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

    OrderStatus status;
    if (!orderStatusFromString(t[5], status)) {
        return false;
    }

    double paymentAmount = 0.0;
    try {
        paymentAmount = stod(t[6]);
    } catch (...) {
        return false;
    }

    PaymentMode mode = PaymentMode::NotSet;
    if (t[7] != "-" && !paymentModeFromString(t[7], mode)) {
        return false;
    }

    Order parsed(t[0], t[1], t[2], decodeToken(t[3]), decodeToken(t[4]));
    parsed.setOrderStatus(status);
    parsed.setPayment(paymentAmount, mode, decodeToken(t[8]));
    out = move(parsed);
    return true;
}

bool OrderFileHandler::parseDetailLine(const string& rawLine, string& outOrderId, OrderItem& outItem) const {
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

    if (t.size() != 4) {
        return false;
    }

    try {
        int productId = stoi(t[1]);
        double qty = stod(t[3]);
        outOrderId = t[0];
        outItem = OrderItem(productId, t[2], qty);
    } catch (...) {
        return false;
    }
    return true;
}

bool OrderFileHandler::writeLinesAtomic(const string& targetPath, const vector<string>& lines) const {
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

bool OrderFileHandler::saveOrder(const Order& order) const {
    if (order.getOrderId().empty()) {
        return false;
    }

    vector<string> lines;
    ifstream in(orderFilePath_);
    if (in.is_open()) {
        string line;
        while (getline(in, line)) {
            Order existing;
            if (parseOrderLine(line, existing) && existing.getOrderId() == order.getOrderId()) {
                continue;
            }
            lines.push_back(line);
        }
        in.close();
    }

    lines.push_back(formatOrderLine(order));
    return writeLinesAtomic(orderFilePath_, lines);
}

bool OrderFileHandler::saveOrderDetails(const Order& order) const {
    if (order.getOrderId().empty() || order.getItems().empty()) {
        return false;
    }

    vector<string> lines;
    ifstream in(orderDetailFilePath_);
    if (in.is_open()) {
        string line;
        while (getline(in, line)) {
            string existingId;
            OrderItem dummy;
            if (parseDetailLine(line, existingId, dummy) && existingId == order.getOrderId()) {
                continue;
            }
            lines.push_back(line);
        }
        in.close();
    }

    for (const auto& item : order.getItems()) {
        lines.push_back(formatDetailLine(order.getOrderId(), item));
    }
    return writeLinesAtomic(orderDetailFilePath_, lines);
}

vector<Order> OrderFileHandler::loadOrders() const {
    vector<Order> orders;
    ifstream in(orderFilePath_);
    if (!in.is_open()) {
        return orders;
    }

    string line;
    while (getline(in, line)) {
        Order o;
        if (parseOrderLine(line, o)) {
            const OrderStatus savedStatus = o.getOrderStatus();
            o.setOrderStatus(OrderStatus::InProgress);
            for (const auto& item : loadOrderDetails(o.getOrderId())) {
                o.addItem(item);
            }
            o.setOrderStatus(savedStatus);
            orders.push_back(move(o));
        }
    }
    return orders;
}

vector<OrderItem> OrderFileHandler::loadOrderDetails(const string& orderId) const {
    vector<OrderItem> items;
    ifstream in(orderDetailFilePath_);
    if (!in.is_open()) {
        return items;
    }

    string line;
    while (getline(in, line)) {
        string id;
        OrderItem item;
        if (parseDetailLine(line, id, item) && id == orderId) {
            items.push_back(move(item));
        }
    }
    return items;
}

optional<Order> OrderFileHandler::findOrderById(const string& orderId) const {
    ifstream in(orderFilePath_);
    if (!in.is_open()) {
        return nullopt;
    }

    string line;
    while (getline(in, line)) {
        Order o;
        if (parseOrderLine(line, o) && o.getOrderId() == orderId) {
            const OrderStatus savedStatus = o.getOrderStatus();
            o.setOrderStatus(OrderStatus::InProgress);
            for (const auto& item : loadOrderDetails(orderId)) {
                o.addItem(item);
            }
            o.setOrderStatus(savedStatus);
            return o;
        }
    }
    return nullopt;
}

bool OrderFileHandler::updateOrderStatus(const string& orderId, OrderStatus newStatus) const {
    ifstream in(orderFilePath_);
    if (!in.is_open()) {
        return false;
    }

    vector<string> lines;
    string line;
    bool found = false;
    while (getline(in, line)) {
        Order existing;
        if (!found && parseOrderLine(line, existing) && existing.getOrderId() == orderId) {
            existing.setOrderStatus(newStatus);
            lines.push_back(formatOrderLine(existing));
            found = true;
        } else {
            lines.push_back(line);
        }
    }
    in.close();

    if (!found) {
        return false;
    }
    return writeLinesAtomic(orderFilePath_, lines);
}

bool OrderFileHandler::removeOrderRecord(const string& orderId) const {
    ifstream in(orderFilePath_);
    if (!in.is_open()) {
        return true;
    }

    vector<string> lines;
    string line;
    while (getline(in, line)) {
        Order existing;
        if (parseOrderLine(line, existing) && existing.getOrderId() == orderId) {
            continue;
        }
        lines.push_back(line);
    }
    in.close();
    return writeLinesAtomic(orderFilePath_, lines);
}

bool OrderFileHandler::removeOrderDetails(const string& orderId) const {
    ifstream in(orderDetailFilePath_);
    if (!in.is_open()) {
        return true;
    }

    vector<string> lines;
    string line;
    while (getline(in, line)) {
        string existingId;
        OrderItem dummy;
        if (parseDetailLine(line, existingId, dummy) && existingId == orderId) {
            continue;
        }
        lines.push_back(line);
    }
    in.close();
    return writeLinesAtomic(orderDetailFilePath_, lines);
}