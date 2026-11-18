#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Order.h"
#include "OrderItem.h"
#include "OrderTypes.h"

using namespace std;

class OrderFileHandler {
public:
    OrderFileHandler(string orderFilePath = "Order.txt",
                     string orderDetailFilePath = "OrderDetail.txt");

    bool saveOrder(const Order& order) const;
    bool saveOrderDetails(const Order& order) const;

    vector<Order> loadOrders() const;
    vector<OrderItem> loadOrderDetails(const string& orderId) const;
    optional<Order> findOrderById(const string& orderId) const;
    bool updateOrderStatus(const string& orderId, OrderStatus newStatus) const;

    bool removeOrderRecord(const string& orderId) const;
    bool removeOrderDetails(const string& orderId) const;
    const string& orderFilePath() const { return orderFilePath_; }
    const string& orderDetailFilePath() const { return orderDetailFilePath_; }

private:
    static string encodeToken(const string& raw);
    static string decodeToken(const string& token);

    string formatOrderLine(const Order& order) const;
    string formatDetailLine(const string& orderId, const OrderItem& item) const;
    bool parseOrderLine(const string& rawLine, Order& out) const;
    bool parseDetailLine(const string& rawLine, string& outOrderId, OrderItem& outItem) const;
    bool writeLinesAtomic(const string& targetPath, const vector<string>& lines) const;

    string orderFilePath_;
    string orderDetailFilePath_;
};
