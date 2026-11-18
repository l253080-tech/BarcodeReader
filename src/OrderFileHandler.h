#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Order.h"
#include "OrderItem.h"
#include "OrderTypes.h"


class OrderFileHandler {
public:
    OrderFileHandler(std::string orderFilePath = "Order.txt",
                     std::string orderDetailFilePath = "OrderDetail.txt");

    bool saveOrder(const Order& order) const;
    bool saveOrderDetails(const Order& order) const;

    std::vector<Order> loadOrders() const;
    std::vector<OrderItem> loadOrderDetails(const std::string& orderId) const;
    std::optional<Order> findOrderById(const std::string& orderId) const;
    bool updateOrderStatus(const std::string& orderId, OrderStatus newStatus) const;

    bool removeOrderRecord(const std::string& orderId) const;
    bool removeOrderDetails(const std::string& orderId) const;
    const std::string& orderFilePath() const { return orderFilePath_; }
    const std::string& orderDetailFilePath() const { return orderDetailFilePath_; }

private:
    static std::string encodeToken(const std::string& raw);
    static std::string decodeToken(const std::string& token);

    std::string formatOrderLine(const Order& order) const;
    std::string formatDetailLine(const std::string& orderId, const OrderItem& item) const;
    bool parseOrderLine(const std::string& rawLine, Order& out) const;
    bool parseDetailLine(const std::string& rawLine, std::string& outOrderId, OrderItem& outItem) const;
    bool writeLinesAtomic(const std::string& targetPath, const std::vector<std::string>& lines) const;

    std::string orderFilePath_;
    std::string orderDetailFilePath_;
};