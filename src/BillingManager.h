#pragma once
#include <ctime>
#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include "Bill.h"
#include "BillFileHandler.h"
#include "Order.h"
#include "OrderFileHandler.h"
#include "OrderManager.h"
#include "OrderTypes.h"
#include "ProductManager.h"

enum class BillingResult {
    Success,
    NoActiveOrder,
    OrderNotPlaced,
    OrderNotEditable,
    EmptyOrder,
    ProductNotFound,
    InsufficientStock,
    InvalidPayment,
    PersistenceFailed,
    InventoryUpdateFailed
};

inline const char* describe(BillingResult r) {
    switch (r) {
    case BillingResult::Success:
        return "Billing completed successfully.";
    case BillingResult::NoActiveOrder:
        return "There is no active order.";
    case BillingResult::OrderNotPlaced:
        return "Place the order before billing.";
    case BillingResult::OrderNotEditable:
        return "This order can no longer be modified.";
    case BillingResult::EmptyOrder:
        return "The order has no items.";
    case BillingResult::ProductNotFound:
        return "One or more products in the order were not found.";
    case BillingResult::InsufficientStock:
        return "Insufficient stock available to complete sale.";
    case BillingResult::InvalidPayment:
        return "Invalid or insufficient payment details.";
    case BillingResult::PersistenceFailed:
        return "Failed to save order or bill records to disk.";
    case BillingResult::InventoryUpdateFailed:
        return "Failed to update product inventory.";
    }
    return "";
}

// Billing + order persistence. Owns the Order.txt / OrderDetail.txt / Bill.txt
// handlers, so everything that must be written to disk goes through here:
//   finalizeBill()      pay: save order, deduct stock, save bill (all-or-nothing)
//   holdCurrentOrder()  put order on hold AND save it as "on-hold"
//   resumeHeldOrder()   bring a held order back for editing
//   cancelCurrentOrder() discard order AND delete its saved records
// On construction, previously saved on-hold orders are restored into OrderManager.
class BillingManager {
public:
    using Clock = std::function<std::time_t()>;

    BillingManager(ProductManager& productManager,
                   OrderManager& orderManager,
                   OrderFileHandler orderFileHandler = OrderFileHandler(),
                   BillFileHandler billFileHandler = BillFileHandler(),
                   Clock clock = [] {
                       return std::time(nullptr);
                   });

    void registerExistingBillId(const std::string& billId);
    // Consumes a bill ID. For on-screen previews use previewBill() instead.
    std::string generateBillId();
    std::optional<Bill> generateBill(const Order& order);
    // Bill for display only: no ID is consumed (bill ID is empty).
    std::optional<Bill> previewBill(const Order& order) const;

    double calculateSubtotal(const Order& order) const;
    double calculateTotalTax(const Order& order) const;
    double calculateTotalDiscount(const Order& order) const;
    double calculateTotalPayable(const Order& order) const;

    void setPaymentDetails(double amount, PaymentMode mode, const std::string& merchant = "");
    bool validatePayment(const Order& order) const;
    bool updatePaymentStatus(Bill& bill, const std::string& status) const;

    BillingResult finalizeBill(std::optional<Bill>& outBill);

    BillingResult holdCurrentOrder();
    OrderResult resumeHeldOrder(const std::string& orderId);
    BillingResult cancelCurrentOrder();

    double getChangeReturned() const {
        return changeReturned_;
    }
    const std::string& getLastPersistenceErrorPath() const { return lastPersistenceErrorPath_; }

private:
    std::optional<Bill> buildBill(const Order& order, const std::string& billId) const;
    void restoreSavedOrders();

    ProductManager& productManager_;
    OrderManager& orderManager_;
    OrderFileHandler orderFileHandler_;
    BillFileHandler billFileHandler_;
    Clock clock_;

    std::unordered_set<std::string> usedBillIds_;
    double paymentAmount_ = 0.0;
    PaymentMode paymentMode_ = PaymentMode::NotSet;
    std::string merchantName_;
    double changeReturned_ = 0.0;
    std::string lastPersistenceErrorPath_;
};