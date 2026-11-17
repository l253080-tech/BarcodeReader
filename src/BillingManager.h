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

using namespace std;

enum class BillingResult {
    Success,
    NoActiveOrder,
    OrderNotPlaced,
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
            return "There is no active order to bill.";
        case BillingResult::OrderNotPlaced:
            return "Place the order before billing.";
        case BillingResult::EmptyOrder:
            return "Cannot generate a bill for an empty order.";
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

class BillingManager {
public:
    using Clock = function<time_t()>;

    BillingManager(ProductManager& productManager,
                   OrderManager& orderManager,
                   OrderFileHandler orderFileHandler = OrderFileHandler(),
                   BillFileHandler billFileHandler = BillFileHandler(),
                   Clock clock = [] {
                       return time(nullptr);
                   });

    void registerExistingBillId(const string& billId);
    string generateBillId();
    optional<Bill> generateBill(const Order& order);

    double calculateSubtotal(const Order& order) const;
    double calculateTotalTax(const Order& order) const;
    double calculateTotalDiscount(const Order& order) const;
    double calculateTotalPayable(const Order& order) const;

    void setPaymentDetails(double amount, PaymentMode mode, const string& merchant = "");
    bool validatePayment(const Order& order) const;
    bool updatePaymentStatus(Bill& bill, const string& status) const;

    BillingResult finalizeBill(optional<Bill>& outBill);

    double getChangeReturned() const {
        return changeReturned_;
    }

private:
    ProductManager& productManager_;
    OrderManager& orderManager_;
    OrderFileHandler orderFileHandler_;
    BillFileHandler billFileHandler_;
    Clock clock_;

    unordered_set<string> usedBillIds_;
    double paymentAmount_ = 0.0;
    PaymentMode paymentMode_ = PaymentMode::NotSet;
    string merchantName_;
    double changeReturned_ = 0.0;
};