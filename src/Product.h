#pragma once
#include <string>

class Product {
public:
    Product();
    Product(int id, const std::string& barcode, const std::string& acronym,
            const std::string& name, const std::string& unit,
            double price, double tax, double discount,
            double quantity, const std::string& barcodePath);

    int getId() const;
    std::string getBarcode() const;
    std::string getAcronym() const;
    std::string getName() const;
    std::string getUnit() const;
    double getPrice() const;
    double getTax() const;
    double getDiscount() const;
    double getQuantity() const;
    std::string getBarcodePath() const;

    void setQuantity(double q);

private:
    int id;
    std::string barcode;
    std::string acronym;
    std::string name;
    std::string unit;
    double price, tax, discount, quantity;
    std::string barcodePath;
};
