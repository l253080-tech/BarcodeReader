#include "ProductManagerCatalog.h"

std::optional<ProductInfo> ProductManagerCatalog::findByBarcode(const std::string& barcode) const {
    auto product = products_.findByBarcode(barcode);
    if (!product) return std::nullopt;

    ProductInfo info;
    info.id = product->getId();
    info.barcode = product->getBarcode();
    info.unitPrice = product->getPrice();
    info.soldByWeight = (product->getUnit() == "weight");
    return info;
}

bool ProductManagerCatalog::checkStock(const std::string& barcode, double quantity) const {
    return products_.checkStock(barcode, quantity);
}
