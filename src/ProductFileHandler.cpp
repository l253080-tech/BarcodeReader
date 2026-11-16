#include "ProductFileHandler.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

ProductFileHandler::ProductFileHandler(const std::string& filePath)
    : filePath(filePath) {}

bool ProductFileHandler::isFileAvailable() const {
    std::ifstream file(filePath);
    return file.is_open();
}

bool ProductFileHandler::parseProductLine(const std::string& rawLine, Product& out) const {
    std::string line = rawLine;
    while (!line.empty() && std::isspace((unsigned char)line.back()))
        line.pop_back();
    if (line.empty() || line.back() != '#') return false;
    line.pop_back();

    std::istringstream stream(line);
    std::vector<std::string> t;
    std::string word;
    while (stream >> word) t.push_back(word);

    if (t.size() < 10) return false;
    size_t n = t.size();

    std::string name = t[3];
    for (size_t i = 4; i < n - 6; ++i) name += " " + t[i];

    try {
        out = Product(std::stoi(t[0]), t[1], t[2], name, t[n - 6],
                      std::stod(t[n - 5]), std::stod(t[n - 4]),
                      std::stod(t[n - 3]), std::stod(t[n - 2]), t[n - 1]);
    } catch (...) {
        return false;
    }
    return true;
}

std::vector<Product> ProductFileHandler::loadProducts() const {
    std::vector<Product> products;
    std::ifstream file(filePath);
    if (!file.is_open()) return products;

    std::string line;
    while (std::getline(file, line)) {
        Product p;
        if (parseProductLine(line, p)) products.push_back(p);
    }
    return products;
}

std::string ProductFileHandler::formatProductLine(const Product& p) const {
    std::ostringstream out;
    out << std::setprecision(10)
        << p.getId() << " " << p.getBarcode() << " " << p.getAcronym() << " "
        << p.getName() << " " << p.getUnit() << " " << p.getPrice() << " "
        << p.getTax() << " " << p.getDiscount() << " " << p.getQuantity() << " "
        << p.getBarcodePath() << " #";
    return out.str();
}

bool ProductFileHandler::writeLines(const std::vector<std::string>& lines) const {
    std::string tempPath = filePath + ".tmp";
    {
        std::ofstream out(tempPath, std::ios::trunc);
        if (!out.is_open()) return false;
        for (const auto& l : lines) out << l << "\n";
        out.flush();
        if (!out.good()) return false;
    }
    std::error_code ec;
    std::filesystem::rename(tempPath, filePath, ec);
    if (ec) {
        std::filesystem::remove(tempPath, ec);
        return false;
    }
    return true;
}

bool ProductFileHandler::saveProducts(const std::vector<Product>& products) const {
    std::vector<std::string> lines;
    for (const auto& p : products) lines.push_back(formatProductLine(p));
    return writeLines(lines);
}

bool ProductFileHandler::updateProductRecord(const Product& product) const {
    std::ifstream in(filePath);
    if (!in.is_open()) return false;

    std::vector<std::string> lines;
    std::string line;
    bool found = false;
    while (std::getline(in, line)) {
        Product existing;
        if (!found && parseProductLine(line, existing) && existing.getId() == product.getId()) {
            lines.push_back(formatProductLine(product));
            found = true;
        } else {
            lines.push_back(line);
        }
    }
    in.close();
    if (!found) return false;
    return writeLines(lines);
}
