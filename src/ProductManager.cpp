#include "ProductManager.h"
#include <algorithm>
#include <sstream>

ProductManager::ProductManager(const ProductFileHandler& handler)
    : handler(handler) {}

bool ProductManager::loadProducts() {
    if (!handler.isFileAvailable()) return false;
    products = handler.loadProducts();
    return true;
}

std::optional<Product> ProductManager::findByBarcode(const std::string& barcode) const {
    auto it = std::find_if(products.begin(), products.end(),
        [&](const Product& p) { return p.getBarcode() == barcode; });
    if (it == products.end()) return std::nullopt;
    return *it;
}

std::optional<Product> ProductManager::findById(int id) const {
    auto it = std::find_if(products.begin(), products.end(),
        [&](const Product& p) { return p.getId() == id; });
    if (it == products.end()) return std::nullopt;
    return *it;
}

const std::vector<Product>& ProductManager::getAllProducts() const {
    return products;
}

bool ProductManager::productExists(const std::string& barcode) const {
    return findByBarcode(barcode).has_value();
}

bool ProductManager::checkStock(const std::string& barcode, double quantity) const {
    if (quantity <= 0) return false;
    auto p = findByBarcode(barcode);
    if (!p) return false;
    return quantity <= p->getQuantity();
}

std::string ProductManager::getProductDescription(const std::string& barcode) const {
    auto p = findByBarcode(barcode);
    if (!p) return "";
    std::ostringstream out;
    out << "Product ID: " << p->getId() << "\n"
        << "Name: " << p->getName() << " (" << p->getAcronym() << ")\n"
        << "Barcode: " << p->getBarcode() << "\n"
        << "Unit: " << p->getUnit() << "\n"
        << "Unit price: PKR " << p->getPrice() << "\n"
        << "Tax: PKR " << p->getTax() << "\n"
        << "Discount per unit: PKR " << p->getDiscount() << "\n"
        << "Available quantity: " << p->getQuantity() << "\n";
    return out.str();
}

bool ProductManager::updateStock(const std::string& barcode, double change) {
    auto it = std::find_if(products.begin(), products.end(),
        [&](const Product& p) { return p.getBarcode() == barcode; });
    if (it == products.end()) return false;

    double oldQty = it->getQuantity();
    double newQty = oldQty + change;
    if (newQty < 0) return false;

    it->setQuantity(newQty);
    if (!handler.updateProductRecord(*it)) {
        it->setQuantity(oldQty);
        return false;
    }
    return true;
}
