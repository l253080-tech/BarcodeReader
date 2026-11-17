#include "Bill.h"
#include <utility>

using namespace std;

Bill::Bill(string billId,
           string customerName,
           string orderId,
           vector<BillItem> items,
           string paymentStatus)
    : billId_(move(billId)),
      customerName_(move(customerName)),
      orderId_(move(orderId)),
      items_(move(items)),
      paymentStatus_(move(paymentStatus)) {
    calculatePayableAmount();
}

string Bill::getProductAcronym() const {
    if (items_.empty()) {
        return "";
    }
    if (items_.size() == 1) {
        return items_.front().productAcronym;
    }

    string joined;
    for (size_t i = 0; i < items_.size(); ++i) {
        if (i > 0) {
            joined += ",";
        }
        joined += items_[i].productAcronym;
    }
    return joined;
}

double Bill::getUnitPriceWithoutTax() const {
    if (items_.size() == 1) {
        return items_.front().unitPriceWithoutTax;
    }
    return totalWithoutTax_;
}

double Bill::getDiscountPerUnit() const {
    if (items_.size() == 1) {
        return items_.front().discountPerUnit;
    }
    return totalDiscount_;
}

double Bill::getProductQuantity() const {
    double totalQty = 0.0;
    for (const auto& item : items_) {
        totalQty += item.quantity;
    }
    return totalQty;
}

void Bill::setItems(const vector<BillItem>& items) {
    items_ = items;
    calculatePayableAmount();
}

void Bill::addItem(const BillItem& item) {
    items_.push_back(item);
    calculatePayableAmount();
}

double Bill::calculatePayableAmount() {
    totalWithoutTax_ = 0.0;
    totalTax_ = 0.0;
    totalDiscount_ = 0.0;

    for (const auto& item : items_) {
        totalWithoutTax_ += item.getLineSubtotal();
        totalTax_        += item.getLineTax();
        totalDiscount_   += item.getLineDiscount();
    }

    totalPayable_ = totalWithoutTax_ + totalTax_ - totalDiscount_;
    if (totalPayable_ < 0.0) {
        totalPayable_ = 0.0;
    }
    return totalPayable_;
}