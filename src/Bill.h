#pragma once
#include <string>
#include <vector>


struct BillItem {
    int productId = 0;
    std::string barcode;
    std::string productAcronym;
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
    Bill(std::string billId,
         std::string customerName,
         std::string orderId,
         std::vector<BillItem> items,
         std::string paymentStatus = "unpaid");

    // Getters
    const std::string& getBillId() const {
        return billId_;
    }

    const std::string& getCustomerName() const {
        return customerName_;
    }

    const std::string& getOrderId() const {
        return orderId_;
    }

    const std::vector<BillItem>& getItems() const {
        return items_;
    }

    // Single-item convenience getters
    std::string getProductAcronym() const;
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

    const std::string& getPaymentStatus() const {
        return paymentStatus_;
    }

    // Setters
    void setBillId(const std::string& id) {
        billId_ = id;
    }

    void setCustomerName(const std::string& name) {
        customerName_ = name;
    }

    void setOrderId(const std::string& id) {
        orderId_ = id;
    }

    void setItems(const std::vector<BillItem>& items);
    void addItem(const BillItem& item);

    void setPaymentStatus(const std::string& status) {
        paymentStatus_ = status;
    }

    double calculatePayableAmount();

private:
    std::string billId_;
    std::string customerName_;
    std::string orderId_;
    std::vector<BillItem> items_;

    double totalWithoutTax_ = 0.0;
    double totalTax_ = 0.0;
    double totalDiscount_ = 0.0;
    double totalPayable_ = 0.0;
    std::string paymentStatus_ = "unpaid";
};