#pragma once
#include <string>

// Shared enums for the order module, plus conversions to/from the exact
// tokens used in Order.txt (no spaces, because fields are space-separated).

enum class OrderStatus { InProgress, OnHold, Paid, Archived };
enum class PaymentMode { NotSet, Cash, Card, Wallet };

inline std::string toString(OrderStatus s) {
    switch (s) {
        case OrderStatus::InProgress: return "in-progress";
        case OrderStatus::OnHold:     return "on-hold";
        case OrderStatus::Paid:       return "paid";
        case OrderStatus::Archived:   return "archived";
    }
    return "in-progress";
}

inline bool orderStatusFromString(const std::string& text, OrderStatus& out) {
    if (text == "in-progress") { out = OrderStatus::InProgress; return true; }
    if (text == "on-hold")     { out = OrderStatus::OnHold;     return true; }
    if (text == "paid")        { out = OrderStatus::Paid;       return true; }
    if (text == "archived")    { out = OrderStatus::Archived;   return true; }
    return false;
}

inline std::string toString(PaymentMode m) {
    switch (m) {
        case PaymentMode::Cash:   return "cash";
        case PaymentMode::Card:   return "card";
        case PaymentMode::Wallet: return "wallet";
        case PaymentMode::NotSet: break;
    }
    return "-";
}

inline bool paymentModeFromString(const std::string& text, PaymentMode& out) {
    if (text == "cash")   { out = PaymentMode::Cash;   return true; }
    if (text == "card")   { out = PaymentMode::Card;   return true; }
    if (text == "wallet") { out = PaymentMode::Wallet; return true; }
    return false;
}
