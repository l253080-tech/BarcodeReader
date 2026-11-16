#include "Order.h"
#include <algorithm>
#include <utility>

Order::Order(std::string orderId, std::string date, std::string time,
             std::string customerName, std::string customerContact)
    : orderId_(std::move(orderId)), date_(std::move(date)), time_(std::move(time)),
      customerName_(std::move(customerName)), customerContact_(std::move(customerContact)) {}

std::vector<OrderItem>::iterator Order::find(int productId) {
    return std::find_if(items_.begin(), items_.end(),
        [&](const OrderItem& i) { return i.getProductId() == productId; });
}

const OrderItem* Order::findItem(int productId) const {
    auto it = std::find_if(items_.begin(), items_.end(),
        [&](const OrderItem& i) { return i.getProductId() == productId; });
    return it == items_.end() ? nullptr : &*it;
}

double Order::getItemQuantity(int productId) const {
    const OrderItem* item = findItem(productId);
    return item ? item->getQuantity() : 0.0;
}

bool Order::addItem(const OrderItem& item) {
    if (!isEditable() || item.getQuantity() <= 0.0) return false;
    auto it = find(item.getProductId());
    if (it != items_.end())
        return it->updateQuantity(it->getQuantity() + item.getQuantity());
    items_.push_back(item);
    return true;
}

bool Order::removeItem(int productId) {
    if (!isEditable()) return false;
    auto it = find(productId);
    if (it == items_.end()) return false;
    items_.erase(it);
    return true;
}

bool Order::updateItemQuantity(int productId, double quantity) {
    if (!isEditable()) return false;
    auto it = find(productId);
    return it != items_.end() && it->updateQuantity(quantity);
}

void Order::clearOrder() {
    if (isEditable()) items_.clear();
}

void Order::setPayment(double amount, PaymentMode mode, const std::string& merchant) {
    paymentAmount_ = amount;
    paymentMode_ = mode;
    merchantName_ = merchant;
}
