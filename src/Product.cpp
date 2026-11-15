#include "Product.h"

Product::Product()
    : id(0), price(0), tax(0), discount(0), quantity(0) {}

Product::Product(int id, const std::string& barcode, const std::string& acronym,
                 const std::string& name, const std::string& unit,
                 double price, double tax, double discount,
                 double quantity, const std::string& barcodePath)
    : id(id), barcode(barcode), acronym(acronym), name(name), unit(unit),
      price(price), tax(tax), discount(discount), quantity(quantity),
      barcodePath(barcodePath) {}

int Product::getId() const { return id; }
std::string Product::getBarcode() const { return barcode; }
std::string Product::getAcronym() const { return acronym; }
std::string Product::getName() const { return name; }
std::string Product::getUnit() const { return unit; }
double Product::getPrice() const { return price; }
double Product::getTax() const { return tax; }
double Product::getDiscount() const { return discount; }
double Product::getQuantity() const { return quantity; }
std::string Product::getBarcodePath() const { return barcodePath; }

void Product::setQuantity(double q) { quantity = q; }
