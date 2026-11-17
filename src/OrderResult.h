#pragma once

// Outcome of an order operation. The GUI maps this to a message / dialog,
// so the backend never has to know anything about the UI.
enum class OrderResult {
    Success,
    NoActiveOrder,
    OrderAlreadyActive,
    OrderNotEditable,
    EmptyOrder,
    ProductNotFound,
    InvalidQuantity,      // <= 0, or fractional quantity for a "count" product
    InsufficientStock,
    ItemNotInOrder,
    HeldOrderNotFound
};

inline const char* describe(OrderResult r) {
    switch (r) {
        case OrderResult::Success:           return "Success";
        case OrderResult::NoActiveOrder:     return "There is no active order.";
        case OrderResult::OrderAlreadyActive:return "Finish, hold or cancel the current order first.";
        case OrderResult::OrderNotEditable:  return "This order can no longer be modified.";
        case OrderResult::EmptyOrder:        return "The order has no items.";
        case OrderResult::ProductNotFound:   return "Product not found for this barcode.";
        case OrderResult::InvalidQuantity:   return "Invalid quantity.";
        case OrderResult::InsufficientStock: return "Not enough stock available.";
        case OrderResult::ItemNotInOrder:    return "This product is not in the order.";
        case OrderResult::HeldOrderNotFound: return "No held order with that ID.";
    }
    return "";
}
