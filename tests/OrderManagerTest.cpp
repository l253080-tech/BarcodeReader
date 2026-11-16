// Standalone test (no Qt needed). Build:
//   g++ -std=c++17 -I../src OrderManagerTest.cpp ../src/Order*.cpp -o order_test
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include "OrderManager.h"

// Fake product storage standing in for Member 1's ProductManager.
class FakeCatalog : public IProductCatalog {
public:
    std::map<std::string, ProductInfo> byBarcode;
    std::map<std::string, double> stock;   // barcode -> available

    std::optional<ProductInfo> findByBarcode(const std::string& b) const override {
        auto it = byBarcode.find(b);
        if (it == byBarcode.end()) return std::nullopt;
        return it->second;
    }
    bool checkStock(const std::string& barcode, double q) const override {
        auto it = stock.find(barcode);
        return it != stock.end() && q <= it->second;
    }
};

#define CHECK(cond) do { if (!(cond)) { std::cerr << "FAILED line " << __LINE__ << ": " #cond "\n"; return 1; } } while (0)

int main() {
    FakeCatalog cat;
    cat.byBarcode["100001"] = {1, "100001", 50.0, false};   // count item
    cat.byBarcode["200002"] = {2, "200002", 120.0, true};   // weight item
    cat.stock["100001"] = 10;
    cat.stock["200002"] = 5.5;

    OrderManager m(cat);

    CHECK(m.addProductToOrder("100001", 1) == OrderResult::NoActiveOrder);
    CHECK(m.createOrder("Ali", "0300") == OrderResult::Success);
    const Order* o = m.getCurrentOrder();
    CHECK(o && o->getOrderId().size() == 8);
    CHECK(o->getOrderStatus() == OrderStatus::InProgress);

    // add one / multiple / same product twice
    CHECK(m.addProductToOrder("100001", 2) == OrderResult::Success);
    CHECK(m.addProductToOrder("200002", 1.5) == OrderResult::Success);
    CHECK(m.addProductToOrder("100001", 3) == OrderResult::Success);
    CHECK(o->getItems().size() == 2);                       // merged, no duplicate
    CHECK(o->getItemQuantity(1) == 5);

    // stock validation uses combined quantity (5 in order + 6 > 10)
    CHECK(m.addProductToOrder("100001", 6) == OrderResult::InsufficientStock);
    CHECK(o->getItemQuantity(1) == 5);
    CHECK(m.addProductToOrder("999999", 1) == OrderResult::ProductNotFound);
    CHECK(m.addProductToOrder("100001", 0) == OrderResult::InvalidQuantity);
    CHECK(m.addProductToOrder("100001", 1.5) == OrderResult::InvalidQuantity);   // count item

    // subtotal: 5*50 + 1.5*120 = 430
    CHECK(std::abs(m.calculateOrderSubtotal() - 430.0) < 1e-9);

    // update / remove
    CHECK(m.updateProductQuantity("100001", 4) == OrderResult::Success);
    CHECK(o->getItemQuantity(1) == 4);
    CHECK(m.updateProductQuantity("100001", 11) == OrderResult::InsufficientStock);
    CHECK(m.removeProductFromOrder("200002") == OrderResult::Success);
    CHECK(m.removeProductFromOrder("200002") == OrderResult::ItemNotInOrder);

    // hold & resume
    std::string id = o->getOrderId();
    CHECK(m.holdOrder() == OrderResult::Success);
    CHECK(m.getCurrentOrder() == nullptr);
    CHECK(m.getHeldOrders().at(id).getOrderStatus() == OrderStatus::OnHold);
    CHECK(m.resumeOrder(id) == OrderResult::Success);
    CHECK(m.getCurrentOrder()->getOrderStatus() == OrderStatus::InProgress);
    CHECK(m.resumeOrder("00000000") == OrderResult::HeldOrderNotFound);

    // place -> locked -> pay
    CHECK(m.placeOrder() == OrderResult::Success);
    CHECK(m.addProductToOrder("100001", 1) == OrderResult::OrderNotEditable);
    auto paid = m.completePayment(200.0, PaymentMode::Cash, "-");
    CHECK(paid && paid->getOrderStatus() == OrderStatus::Paid);
    CHECK(m.getCurrentOrder() == nullptr);

    // cancel
    CHECK(m.createOrder("Sara") == OrderResult::Success);
    CHECK(m.addProductToOrder("100001", 1) == OrderResult::Success);
    CHECK(m.createOrder("Other") == OrderResult::OrderAlreadyActive);
    CHECK(m.cancelOrder() == OrderResult::Success);
    CHECK(m.cancelOrder() == OrderResult::NoActiveOrder);
    CHECK(m.placeOrder() == OrderResult::NoActiveOrder);

    // unique IDs, last two digits = day
    OrderIdGenerator g;
    g.registerExistingId("00000115");
    std::set<std::string> ids;
    for (int i = 0; i < 1000; ++i) {
        std::string x = g.generate(15);
        CHECK(x.size() == 8 && x.substr(6) == "15");
        CHECK(ids.insert(x).second);
        CHECK(x != "00000115");
    }

    std::cout << "All order module tests passed\n";
    return 0;
}
