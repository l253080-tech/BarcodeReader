#include "OrderManager.h"
#include <cmath>
#include <cstdio>
#include <utility>

namespace {
std::tm toLocal(std::time_t t) {
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    return tm;
}
}

OrderManager::OrderManager(const IProductCatalog& catalog, OrderIdGenerator idGenerator, Clock clock)
    : catalog_(catalog), idGenerator_(std::move(idGenerator)), clock_(std::move(clock)) {}

OrderResult OrderManager::createOrder(const std::string& customerName, const std::string& customerContact) {
    // An untouched empty order may be replaced; one with items must be dealt with first.
    if (current_ && !current_->getItems().empty()) return OrderResult::OrderAlreadyActive;

    std::tm tm = toLocal(clock_());
    char date[48], time[48];
    std::snprintf(date, sizeof date, "%02d-%02d-%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    std::snprintf(time, sizeof time, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

    current_.emplace(idGenerator_.generate(tm.tm_mday), date, time, customerName, customerContact);
    return OrderResult::Success;
}

OrderResult OrderManager::validateQuantity(const ProductInfo& product, double quantity) const {
    if (!(quantity > 0.0)) return OrderResult::InvalidQuantity;
    // "count" products can only be sold in whole units
    if (!product.soldByWeight && std::fabs(quantity - std::round(quantity)) > 1e-9)
        return OrderResult::InvalidQuantity;
    return OrderResult::Success;
}

OrderResult OrderManager::addProductToOrder(const std::string& barcode, double quantity) {
    if (!hasActiveOrder()) return OrderResult::NoActiveOrder;
    if (!current_->isEditable()) return OrderResult::OrderNotEditable;

    auto product = catalog_.findByBarcode(barcode);
    if (!product) return OrderResult::ProductNotFound;

    OrderResult valid = validateQuantity(*product, quantity);
    if (valid != OrderResult::Success) return valid;

    double combined = current_->getItemQuantity(product->id) + quantity;
    if (!catalog_.checkStock(product->barcode, combined)) return OrderResult::InsufficientStock;

    current_->addItem(OrderItem(product->id, product->barcode, quantity));
    return OrderResult::Success;
}

OrderResult OrderManager::removeProductFromOrder(const std::string& barcode) {
    if (!hasActiveOrder()) return OrderResult::NoActiveOrder;
    if (!current_->isEditable()) return OrderResult::OrderNotEditable;

    auto product = catalog_.findByBarcode(barcode);
    if (!product) return OrderResult::ProductNotFound;
    return current_->removeItem(product->id) ? OrderResult::Success : OrderResult::ItemNotInOrder;
}

OrderResult OrderManager::updateProductQuantity(const std::string& barcode, double quantity) {
    if (!hasActiveOrder()) return OrderResult::NoActiveOrder;
    if (!current_->isEditable()) return OrderResult::OrderNotEditable;

    auto product = catalog_.findByBarcode(barcode);
    if (!product) return OrderResult::ProductNotFound;
    if (!current_->findItem(product->id)) return OrderResult::ItemNotInOrder;

    OrderResult valid = validateQuantity(*product, quantity);
    if (valid != OrderResult::Success) return valid;
    if (!catalog_.checkStock(product->barcode, quantity)) return OrderResult::InsufficientStock;

    current_->updateItemQuantity(product->id, quantity);
    return OrderResult::Success;
}

double OrderManager::calculateOrderSubtotal() const {
    if (!current_) return 0.0;
    double subtotal = 0.0;
    for (const OrderItem& item : current_->getItems())
        if (auto product = catalog_.findByBarcode(item.getBarcode()))
            subtotal += product->unitPrice * item.getQuantity();
    return subtotal;
}

OrderResult OrderManager::placeOrder() {
    if (!hasActiveOrder()) return OrderResult::NoActiveOrder;
    if (!current_->isEditable()) return OrderResult::OrderNotEditable;
    if (current_->getItems().empty()) return OrderResult::EmptyOrder;
    current_->lock();
    return OrderResult::Success;
}

std::optional<Order> OrderManager::completePayment(double amount, PaymentMode mode, const std::string& merchant) {
    if (!current_ || !current_->isLocked()) return std::nullopt;   // must be placed first
    current_->setPayment(amount, mode, merchant);
    current_->setOrderStatus(OrderStatus::Paid);
    Order done = std::move(*current_);
    current_.reset();
    return done;
}

OrderResult OrderManager::holdOrder() {
    if (!hasActiveOrder()) return OrderResult::NoActiveOrder;
    if (!current_->isEditable()) return OrderResult::OrderNotEditable;
    if (current_->getItems().empty()) return OrderResult::EmptyOrder;

    current_->setOrderStatus(OrderStatus::OnHold);
    std::string id = current_->getOrderId();
    held_.emplace(id, std::move(*current_));
    current_.reset();
    return OrderResult::Success;
}

OrderResult OrderManager::resumeOrder(const std::string& orderId) {
    auto it = held_.find(orderId);
    if (it == held_.end()) return OrderResult::HeldOrderNotFound;
    if (current_ && !current_->getItems().empty()) return OrderResult::OrderAlreadyActive;

    it->second.setOrderStatus(OrderStatus::InProgress);
    it->second.unlock();
    current_ = std::move(it->second);
    held_.erase(it);
    return OrderResult::Success;
}

OrderResult OrderManager::cancelOrder() {
    if (!hasActiveOrder()) return OrderResult::NoActiveOrder;
    current_.reset();
    return OrderResult::Success;
}
