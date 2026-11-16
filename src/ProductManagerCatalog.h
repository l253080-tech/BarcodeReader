#pragma once
#include "IProductCatalog.h"
#include "ProductManager.h"

// Adapter: lets the order module use Member 1's ProductManager through the
// IProductCatalog interface. Neither side needs to know about the other.
class ProductManagerCatalog : public IProductCatalog {
public:
    explicit ProductManagerCatalog(const ProductManager& products) : products_(products) {}

    std::optional<ProductInfo> findByBarcode(const std::string& barcode) const override;
    bool checkStock(const std::string& barcode, double quantity) const override;

private:
    const ProductManager& products_;
};
