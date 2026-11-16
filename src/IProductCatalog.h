#pragma once
#include <optional>
#include <string>

// What the order module needs to know about a product.
struct ProductInfo {
    int id = 0;
    std::string barcode;
    double unitPrice = 0.0;
    bool soldByWeight = false;   // false = "count" (whole units only)
};

// Abstraction over Member 1's ProductManager (Dependency Inversion).
// OrderManager depends only on this interface, so it can be tested with a fake
// catalog and works unchanged if the product storage changes.
// Member 1's ProductManager is plugged in through ProductManagerCatalog (adapter).
class IProductCatalog {
public:
    virtual ~IProductCatalog() = default;
    virtual std::optional<ProductInfo> findByBarcode(const std::string& barcode) const = 0;
    // true if 'quantity' units of the product with this barcode can be sold from available stock
    virtual bool checkStock(const std::string& barcode, double quantity) const = 0;
};
