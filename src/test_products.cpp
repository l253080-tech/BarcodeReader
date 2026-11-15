#include <fstream>
#include <iostream>
#include "ProductManager.h"

int main() {
    // 1. missing file
    ProductManager missing{ProductFileHandler("DoesNotExist.txt")};
    std::cout << "missing file loads: " << missing.loadProducts() << "\n";

    // 2. file with bad lines
    {
        std::ofstream f("Bad.txt");
        f << "1 123456 RICE5KG Basmati Rice count 1850 120 50 40 a.png #\n";
        f << "this is garbage\n";
        f << "2 234567 MILK1L Milk count abc 15 10 100 a.png #\n";
        f << "3 345678 SUGAR Sugar weight 180 0 5 25.5 a.png\n";
        f << "4 456789 TEA Tea count 90 0 0 10 a.png #\n";
    }
    ProductManager bad{ProductFileHandler("Bad.txt")};
    bad.loadProducts();
    std::cout << "valid products in Bad.txt: " << bad.getAllProducts().size() << "\n";

    // 3. updating must keep the bad lines
    bad.updateStock("123456", -5);
    std::ifstream in("Bad.txt");
    std::string line;
    int lines = 0;
    while (std::getline(in, line)) ++lines;
    std::cout << "lines after update: " << lines << "\n";
}
