#include "productdetails.h"

#include "Product.h"

#include <QDir>

QString formatProductDetails(const Product &product, const QString &dataDirectory)
{
    const QString relativePath = QString::fromStdString(product.getBarcodePath());
    const QString resolvedPath = relativePath.isEmpty()
        ? QStringLiteral("(not set)")
        : QDir(dataDirectory).filePath(relativePath);

    return QStringLiteral(
        "Product ID: %1\nBarcode: %2\nAcronym: %3\nName: %4\nUnit: %5\n"
        "Unit price: PKR %6\nTax per unit: PKR %7\nDiscount per unit: PKR %8\n"
        "Available quantity: %9\nBarcode image: %10")
        .arg(product.getId())
        .arg(QString::fromStdString(product.getBarcode()))
        .arg(QString::fromStdString(product.getAcronym()))
        .arg(QString::fromStdString(product.getName()))
        .arg(QString::fromStdString(product.getUnit()))
        .arg(product.getPrice(), 0, 'f', 2)
        .arg(product.getTax(), 0, 'f', 2)
        .arg(product.getDiscount(), 0, 'f', 2)
        .arg(product.getQuantity(), 0, 'f', 2)
        .arg(resolvedPath);
}
