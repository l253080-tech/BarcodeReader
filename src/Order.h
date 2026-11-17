#pragma once
#include <string>
#include <vector>
#include "OrderItem.h"
#include "OrderTypes.h"

// A customer order: header data (matches one Order.txt row) plus its items
// (matches the OrderDetail.txt rows). It only holds and guards its own data;
// stock checks, ID generation and workflow live elsewhere.
class Order {
public:
    Order() = default;
    Order(std::string orderId, std::string date, std::string time,
          std::string customerName, std::string customerContact = "");

    // ---- items ----
    // Adds the item; if the product is already in the order its quantity is increased.
    // Returns false if the order is not editable or quantity <= 0.
    bool addItem(const OrderItem& item);
    bool removeItem(int productId);
    bool updateItemQuantity(int productId, double quantity);
    void clearOrder();

    const std::vector<OrderItem>& getItems() const { return items_; }
    const OrderItem* findItem(int productId) const;
    double getItemQuantity(int productId) const;   // 0 if absent

    // ---- header ----
    const std::string& getOrderId() const         { return orderId_; }
    const std::string& getOrderDate() const       { return date_; }
    const std::string& getOrderTime() const       { return time_; }
    const std::string& getCustomerName() const    { return customerName_; }
    const std::string& getCustomerContact() const { return customerContact_; }
    OrderStatus getOrderStatus() const            { return status_; }
    double getPaymentAmount() const               { return paymentAmount_; }
    PaymentMode getPaymentMode() const            { return paymentMode_; }
    const std::string& getMerchantName() const    { return merchantName_; }

    void setCustomerName(const std::string& n)    { customerName_ = n; }
    void setCustomerContact(const std::string& c) { customerContact_ = c; }
    void setOrderStatus(OrderStatus s)            { status_ = s; }
    void setPayment(double amount, PaymentMode mode, const std::string& merchant);

    // A locked order (placed for billing) rejects item changes.
    void lock()           { locked_ = true; }
    void unlock()         { locked_ = false; }
    bool isLocked() const { return locked_; }
    bool isEditable() const { return status_ == OrderStatus::InProgress && !locked_; }

private:
    std::vector<OrderItem>::iterator find(int productId);

    std::string orderId_;
    std::string date_;
    std::string time_;
    std::string customerName_;
    std::string customerContact_;
    OrderStatus status_ = OrderStatus::InProgress;
    double paymentAmount_ = 0.0;
    PaymentMode paymentMode_ = PaymentMode::NotSet;
    std::string merchantName_;
    std::vector<OrderItem> items_;
    bool locked_ = false;
};
