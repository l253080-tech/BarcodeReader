#pragma once
#include <string>
#include <vector>

using namespace std;

struct BillItem {
    int productId = 0;
    string barcode;
    string productAcronym;
    double unitPriceWithoutTax = 0.0;
    double taxPerUnit = 0.0;
    double discountPerUnit = 0.0;
    double quantity = 0.0;

    double getLineSubtotal() const {
        return unitPriceWithoutTax * quantity;
    }

    double getLineTax() const {
        return taxPerUnit * quantity;
    }

    double getLineDiscount() const {
        return discountPerUnit * quantity;
    }

    double getLinePayable() const {
        return getLineSubtotal() + getLineTax() - getLineDiscount();
    }
};

class Bill {
public:
    Bill() = default;
    Bill(string billId,
         string customerName,
         string orderId,
         vector<BillItem> items,
         string paymentStatus = "unpaid");

    // Getters
    const string& getBillId() const {
        return billId_;
    }

    const string& getCustomerName() const {
        return customerName_;
    }

    const string& getOrderId() const {
        return orderId_;
    }

    const vector<BillItem>& getItems() const {
        return items_;
    }

    // Single-item convenience getters
    string getProductAcronym() const;
    double getUnitPriceWithoutTax() const;
    double getDiscountPerUnit() const;
    double getProductQuantity() const;

    double getTotalWithoutTax() const {
        return totalWithoutTax_;
    }

    double getTotalTax() const {
        return totalTax_;
    }

    double getTotalDiscount() const {
        return totalDiscount_;
    }

    double getTotalPayable() const {
        return totalPayable_;
    }

    const string& getPaymentStatus() const {
        return paymentStatus_;
    }

    // Setters
    void setBillId(const string& id) {
        billId_ = id;
    }

    void setCustomerName(const string& name) {
        customerName_ = name;
    }

    void setOrderId(const string& id) {
        orderId_ = id;
    }

    void setItems(const vector<BillItem>& items);
    void addItem(const BillItem& item);

    void setPaymentStatus(const string& status) {
        paymentStatus_ = status;
    }

    double calculatePayableAmount();

private:
    string billId_;
    string customerName_;
    string orderId_;
    vector<BillItem> items_;

    double totalWithoutTax_ = 0.0;
    double totalTax_ = 0.0;
    double totalDiscount_ = 0.0;
    double totalPayable_ = 0.0;
    string paymentStatus_ = "unpaid";
};