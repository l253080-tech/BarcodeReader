#include "OrderItem.h"
#include <utility>

OrderItem::OrderItem(int productId, std::string barcode, double quantity)
    : productId_(productId), barcode_(std::move(barcode)), quantity_(quantity) {}

bool OrderItem::updateQuantity(double quantity) {
    if (quantity <= 0.0) return false;
    quantity_ = quantity;
    return true;
}
