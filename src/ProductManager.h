#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Product.h"
#include "ProductFileHandler.h"

class ProductManager {
public:
    explicit ProductManager(const ProductFileHandler& handler);

    bool loadProducts();
    std::optional<Product> findByBarcode(const std::string& barcode) const;
    std::optional<Product> findById(int id) const;
    const std::vector<Product>& getAllProducts() const;
    bool productExists(const std::string& barcode) const;

    bool checkStock(const std::string& barcode, double quantity) const;
    bool updateStock(const std::string& barcode, double change);
    std::string getProductDescription(const std::string& barcode) const;

private:
    ProductFileHandler handler;
    std::vector<Product> products;
};
