#pragma once
#include <string>
#include <vector>
#include "Product.h"

class ProductFileHandler {
public:
    explicit ProductFileHandler(const std::string& filePath);

    bool isFileAvailable() const;
    std::vector<Product> loadProducts() const;
    bool parseProductLine(const std::string& line, Product& out) const;
    bool saveProducts(const std::vector<Product>& products) const;
    bool updateProductRecord(const Product& product) const;
    const std::string& filePath() const { return filePath_; }

private:
    std::string formatProductLine(const Product& p) const;
    bool writeLines(const std::vector<std::string>& lines) const;

    std::string filePath_;
};
