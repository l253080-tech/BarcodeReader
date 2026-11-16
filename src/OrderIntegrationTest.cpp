// Order module + Member 1's real ProductManager + Product.txt.
// Product.txt must be in the working directory (CMake copies it next to the executable).
#include <cmath>
#include <iostream>
#include "OrderManager.h"
#include "ProductManagerCatalog.h"

#define CHECK(cond) do { if (!(cond)) { std::cerr << "FAILED line " << __LINE__ << ": " #cond "\n"; return 1; } } while (0)

int main() {
    ProductManager products{ProductFileHandler("Product.txt")};
    CHECK(products.loadProducts());

    ProductManagerCatalog catalog(products);
    OrderManager manager(catalog);

    CHECK(manager.createOrder("Ali") == OrderResult::Success);

    // 123456 = Basmati Rice (count, stock 40, PKR 1850); 345678 = Loose Sugar (weight, stock 25.5, PKR 180)
    CHECK(manager.addProductToOrder("123456", 2) == OrderResult::Success);
    CHECK(manager.addProductToOrder("123456", 39) == OrderResult::InsufficientStock);   // 2 + 39 > 40
    CHECK(manager.addProductToOrder("123456", 38) == OrderResult::Success);             // exactly 40
    CHECK(manager.getCurrentOrder()->getItems().size() == 1);
    CHECK(manager.addProductToOrder("345678", 2.5) == OrderResult::Success);
    CHECK(manager.addProductToOrder("123456", 1.5) == OrderResult::InvalidQuantity);    // count item
    CHECK(manager.addProductToOrder("999999", 1) == OrderResult::ProductNotFound);

    // 40 * 1850 + 2.5 * 180
    CHECK(std::abs(manager.calculateOrderSubtotal() - 74450.0) < 1e-9);

    CHECK(manager.updateProductQuantity("123456", 41) == OrderResult::InsufficientStock);
    CHECK(manager.updateProductQuantity("123456", 3) == OrderResult::Success);
    CHECK(manager.removeProductFromOrder("345678") == OrderResult::Success);
    CHECK(manager.placeOrder() == OrderResult::Success);

    std::cout << "Order + ProductManager integration tests passed\n";
    return 0;
}
