#pragma once
#include <string>

// One product line inside an order. Quantity is a double because some
// products are sold by weight.
class OrderItem {
public:
    OrderItem() = default;
    OrderItem(int productId, std::string barcode, double quantity);

    int getProductId() const                { return productId_; }
    const std::string& getBarcode() const   { return barcode_; }
    double getQuantity() const              { return quantity_; }

    void setProductId(int id)                 { productId_ = id; }
    void setBarcode(const std::string& code)  { barcode_ = code; }

    // Replaces the quantity. Returns false (and changes nothing) if quantity <= 0.
    bool updateQuantity(double quantity);

private:
    int productId_ = 0;
    std::string barcode_;
    double quantity_ = 0.0;
};
