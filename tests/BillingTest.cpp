#include <cassert>
#include <filesystem>
#include <iostream>
#include <optional>
#include "Bill.h"
#include "BillFileHandler.h"
#include "BillingManager.h"
#include "OrderFileHandler.h"
#include "OrderManager.h"
#include "ProductFileHandler.h"
#include "ProductManager.h"
#include "ProductManagerCatalog.h"

using namespace std;

int main() {
    filesystem::remove("Test_Products.txt");
    filesystem::remove("Test_Order.txt");
    filesystem::remove("Test_OrderDetail.txt");
    filesystem::remove("Test_Bill.txt");

    ProductFileHandler prodFile("Test_Products.txt");
    vector<Product> initialProducts = {
        Product(1, "123456", "RICE5KG", "Basmati Rice 5kg", "count", 1850.0, 120.0, 50.0, 40.0, "barcodes/123456.png"),
        Product(2, "234567", "MILK1L", "Olpers Milk", "count", 310.0, 15.0, 10.0, 100.0, "barcodes/234567.png"),
        Product(3, "345678", "SUGAR", "Loose Sugar", "weight", 180.0, 0.0, 5.0, 25.5, "barcodes/345678.png")
    };
    assert(prodFile.saveProducts(initialProducts));

    ProductManager productManager(prodFile);
    assert(productManager.loadProducts());

    ProductManagerCatalog catalog(productManager);
    OrderManager orderManager(catalog);

    OrderFileHandler orderFiles("Test_Order.txt", "Test_OrderDetail.txt");
    BillFileHandler billFiles("Test_Bill.txt");

    BillingManager billingManager(productManager, orderManager, orderFiles, billFiles);

    // 1. Create order and add multiple products (count + weight)
    assert(orderManager.createOrder("Ali Khan", "03001234567") == OrderResult::Success);
    assert(orderManager.addProductToOrder("123456", 2.0) == OrderResult::Success);
    assert(orderManager.addProductToOrder("345678", 1.5) == OrderResult::Success);

    const Order* activeOrder = orderManager.getCurrentOrder();
    assert(activeOrder != nullptr);

    // Expected:
    // Subtotal = (1850 * 2) + (180 * 1.5) = 3700 + 270 = 3970
    // Tax      = (120 * 2) + (0 * 1.5)    = 240
    // Discount = (50 * 2) + (5 * 1.5)     = 100 + 7.5 = 107.5
    // Payable  = 3970 + 240 - 107.5       = 4102.5
    assert(billingManager.calculateSubtotal(*activeOrder) == 3970.0);
    assert(billingManager.calculateTotalTax(*activeOrder) == 240.0);
    assert(billingManager.calculateTotalDiscount(*activeOrder) == 107.5);
    assert(billingManager.calculateTotalPayable(*activeOrder) == 4102.5);

    // 2. Finalize fails if order is not placed yet
    optional<Bill> generatedBill;
    billingManager.setPaymentDetails(5000.0, PaymentMode::Cash, "");
    assert(billingManager.finalizeBill(generatedBill) == BillingResult::OrderNotPlaced);

    // 3. Place order, test insufficient payment, then valid payment
    assert(orderManager.placeOrder() == OrderResult::Success);

    billingManager.setPaymentDetails(4000.0, PaymentMode::Cash, "");
    assert(billingManager.finalizeBill(generatedBill) == BillingResult::InvalidPayment);

    billingManager.setPaymentDetails(4500.0, PaymentMode::Card, "HBL POS");
    assert(billingManager.finalizeBill(generatedBill) == BillingResult::Success);
    assert(generatedBill.has_value());
    assert(generatedBill->getBillId().size() == 9);
    assert(generatedBill->getTotalPayable() == 4102.5);
    assert(generatedBill->getPaymentStatus() == "paid");
    assert(billingManager.getChangeReturned() == 397.5);

    // 4. Verify persistence in Order.txt, OrderDetail.txt, Bill.txt
    auto savedOrders = orderFiles.loadOrders();
    assert(savedOrders.size() == 1);
    assert(savedOrders[0].getItems().size() == 2);
    assert(savedOrders[0].getOrderStatus() == OrderStatus::Paid);

    auto savedBills = billFiles.loadBills();
    assert(savedBills.size() == 1);
    assert(savedBills[0].getBillId() == generatedBill->getBillId());
    assert(savedBills[0].getItems().size() == 2);

    // 5. Verify inventory was deducted in Product.txt
    auto updatedRice = productManager.findByBarcode("123456");
    auto updatedSugar = productManager.findByBarcode("345678");
    assert(updatedRice.has_value() && updatedRice->getQuantity() == 38.0);
    assert(updatedSugar.has_value() && updatedSugar->getQuantity() == 24.0);

    filesystem::remove("Test_Products.txt");
    filesystem::remove("Test_Order.txt");
    filesystem::remove("Test_OrderDetail.txt");
    filesystem::remove("Test_Bill.txt");

    cout << "All BillingTest assertions passed successfully!\n";
    return 0;
}