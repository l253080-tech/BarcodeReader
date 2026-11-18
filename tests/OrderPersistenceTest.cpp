#include "BillFileHandler.h"
#include "BillingManager.h"
#include "OrderFileHandler.h"
#include "OrderManager.h"
#include "ProductFileHandler.h"
#include "ProductManager.h"
#include "ProductManagerCatalog.h"

#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static void require(bool condition, const std::string &message)
{
    if (!condition) throw std::runtime_error(message);
}

static std::vector<std::string> linesIn(const fs::path &path)
{
    std::ifstream in(path);
    require(in.is_open(), "Unable to read " + path.string());
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line))
        if (!line.empty()) lines.push_back(line);
    return lines;
}

int main(int argc, char **argv)
{
    try {
        require(argc > 0, "Executable path is missing");
        const fs::path executableDir = fs::absolute(argv[0]).parent_path();
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const char *configuredDataDir = std::getenv("BARCODEREADER_PERSISTENCE_DATA_DIR");
        const bool useConfiguredDataDir = configuredDataDir && *configuredDataDir;
        const fs::path dataDir = useConfiguredDataDir
            ? fs::absolute(configuredDataDir)
            : fs::temp_directory_path() / ("BarcodeReaderPersistenceSmoke_" + std::to_string(stamp));
        fs::create_directories(dataDir);
        if (!fs::exists(dataDir / "Product.txt"))
            fs::copy_file(executableDir / "Product.txt", dataDir / "Product.txt");

        const fs::path productPath = dataDir / "Product.txt";
        const fs::path orderPath = dataDir / "Order.txt";
        const fs::path detailPath = dataDir / "OrderDetail.txt";
        const fs::path billPath = dataDir / "Bill.txt";
        for (const auto &path : {orderPath, detailPath, billPath}) {
            if (fs::exists(path)) continue;
            std::ofstream create(path);
            require(create.is_open(), "Unable to create " + path.string());
            create.flush();
            require(create.good(), "Unable to flush " + path.string());
            create.close();
            require(!create.fail(), "Unable to close " + path.string());
        }

        OrderFileHandler orderFiles(orderPath.string(), detailPath.string());
        BillFileHandler billFiles(billPath.string());
        std::string orderId;
        double initialStock = 0.0;

        // First process lifetime: create, add, hold, and persist.
        {
            ProductFileHandler productFile(productPath.string());
            ProductManager products(productFile);
            require(products.loadProducts(), "Product data did not load");
            initialStock = products.findByBarcode("123456")->getQuantity();
            ProductManagerCatalog catalog(products);
            OrderManager orders(catalog);
            BillingManager billing(products, orders, orderFiles, billFiles);

            require(orders.createOrder("Demo Customer", "555-0100") == OrderResult::Success,
                    "Could not create the order");
            require(orders.addProductToOrder("123456", 2.0) == OrderResult::Success,
                    "Could not add sample product");
            orderId = orders.getCurrentOrder()->getOrderId();
            require(orders.holdOrder() == OrderResult::Success, "Could not hold order");
            const Order &held = orders.getHeldOrders().at(orderId);
            require(orderFiles.saveOrder(held), "Could not persist held order");
            require(orderFiles.saveOrderDetails(held), "Could not persist held order items");
        }

        // Second process lifetime: reload and prove the held order can be resumed.
        {
            ProductFileHandler productFile(productPath.string());
            ProductManager products(productFile);
            require(products.loadProducts(), "Product data did not reload");
            ProductManagerCatalog catalog(products);
            OrderManager orders(catalog);
            BillingManager billing(products, orders, orderFiles, billFiles);

            const auto persistedOrders = orderFiles.loadOrders();
            auto heldIt = std::find_if(persistedOrders.begin(), persistedOrders.end(),
                [&](const Order &order) {
                    return order.getOrderId() == orderId
                        && order.getOrderStatus() == OrderStatus::OnHold
                        && order.getItems().size() == 1
                        && std::abs(order.getItems().front().getQuantity() - 2.0) < 1e-9;
                });
            require(heldIt != persistedOrders.end(), "Held order/items were not restored from disk");
            require(orders.restoreHeldOrder(*heldIt), "OrderManager rejected restored held order");
            require(orders.getHeldOrders().count(orderId) == 1,
                    "Restored order is not in the Resume held list");
            require(orders.resumeOrder(orderId) == OrderResult::Success,
                    "Could not resume persisted order");
            require(orderFiles.saveOrder(*orders.getCurrentOrder()), "Could not persist resumed status");
            require(orderFiles.saveOrderDetails(*orders.getCurrentOrder()), "Could not persist resumed items");

            require(orders.placeOrder() == OrderResult::Success, "Could not place order");
            require(orderFiles.saveOrder(*orders.getCurrentOrder()), "Could not persist placed status");
            require(orderFiles.saveOrderDetails(*orders.getCurrentOrder()), "Could not persist placed items");
            const double due = billing.calculateTotalPayable(*orders.getCurrentOrder());
            billing.setPaymentDetails(due + 10.0, PaymentMode::Cash);
            std::optional<Bill> bill;
            require(billing.finalizeBill(bill) == BillingResult::Success && bill.has_value(),
                    "Payment/bill finalization failed");

            const auto finalOrders = orderFiles.loadOrders();
            const auto finalOrder = std::find_if(finalOrders.begin(), finalOrders.end(),
                [&](const Order &order) { return order.getOrderId() == orderId; });
            require(finalOrder != finalOrders.end() && finalOrder->getOrderStatus() == OrderStatus::Paid,
                    "Paid order line was not updated");
            require(std::count_if(finalOrders.begin(), finalOrders.end(),
                [&](const Order &order) { return order.getOrderId() == orderId; }) == 1,
                "Order.txt contains duplicate order rows");
            require(orderFiles.loadOrderDetails(orderId).size() == 1,
                    "OrderDetail.txt contains missing or duplicate item rows");
            const auto bills = billFiles.loadBills();
            require(std::count_if(bills.begin(), bills.end(), [&](const Bill &savedBill) {
                return savedBill.getOrderId() == orderId;
            }) == 1, "Bill.txt does not contain exactly one bill for this order");
            const auto product = products.findByBarcode("123456");
            require(product && std::abs(product->getQuantity() - (initialStock - 2.0)) < 1e-9,
                    "Product stock was not reduced after payment");
        }

        std::cout << "RESTART_RESTORE_OK order=" << orderId << " held item count=1\n";
        for (const auto &entry : std::vector<std::pair<std::string, fs::path>>{
                 {"Order.txt", orderPath}, {"OrderDetail.txt", detailPath}, {"Bill.txt", billPath}}) {
            std::cout << "FILE " << entry.first << " " << fs::absolute(entry.second).string() << "\n";
            for (const auto &line : linesIn(entry.second))
                std::cout << line << "\n";
        }
        std::cout << "PRODUCT_FILE " << fs::absolute(productPath).string() << "\n";
        for (const auto &line : linesIn(productPath)) std::cout << line << "\n";
        if (!useConfiguredDataDir && std::getenv("BARCODEREADER_KEEP_PERSISTENCE_TEST_FILES") == nullptr)
            fs::remove_all(dataDir);
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "PERSISTENCE_TEST_FAILED: " << error.what() << "\n";
        return 1;
    }
}
