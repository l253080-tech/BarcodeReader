#pragma once
#include <ctime>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include "IProductCatalog.h"
#include "Order.h"
#include "OrderIdGenerator.h"
#include "OrderResult.h"

// Order workflow: create -> scan/add products -> place -> pay, plus hold/resume/cancel.
// Products are resolved and stock-checked through IProductCatalog; this class
// does no file I/O, billing maths or GUI work.
class OrderManager {
public:
    using Clock = std::function<std::time_t()>;

    explicit OrderManager(const IProductCatalog& catalog,
                          OrderIdGenerator idGenerator = OrderIdGenerator(),
                          Clock clock = [] { return std::time(nullptr); });

    // Call for each ID loaded from Order.txt so new IDs never collide.
    void registerExistingOrderId(const std::string& id) { idGenerator_.registerExistingId(id); }

    OrderResult createOrder(const std::string& customerName, const std::string& customerContact = "");

    // 'quantity' is the amount requested now; stock is checked against
    // (quantity already in the order + quantity).
    OrderResult addProductToOrder(const std::string& barcode, double quantity);
    OrderResult removeProductFromOrder(const std::string& barcode);
    // 'quantity' is the new total for the product (also stock-checked).
    OrderResult updateProductQuantity(const std::string& barcode, double quantity);

    const Order* getCurrentOrder() const { return current_ ? &*current_ : nullptr; }
    // Sum of unit price * quantity, before tax and discount. 0 if no order.
    double calculateOrderSubtotal() const;

    // Locks the order so it is ready for billing (Member 4).
    OrderResult placeOrder();
    // After payment: marks the order paid, releases it from the manager and returns it for saving.
    std::optional<Order> completePayment(double amount, PaymentMode mode, const std::string& merchant);

    OrderResult holdOrder();
    OrderResult resumeOrder(const std::string& orderId);
    // Restores a persisted on-hold order at startup without changing its contents.
    bool restoreHeldOrder(const Order& order);
    OrderResult cancelOrder();                     // discards the current order
    const std::map<std::string, Order>& getHeldOrders() const { return held_; }

private:
    OrderResult validateQuantity(const ProductInfo& product, double quantity) const;
    bool hasActiveOrder() const { return current_.has_value(); }

    const IProductCatalog& catalog_;
    OrderIdGenerator idGenerator_;
    Clock clock_;
    std::optional<Order> current_;
    std::map<std::string, Order> held_;
};
