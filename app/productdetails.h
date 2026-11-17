#ifndef PRODUCTDETAILS_H
#define PRODUCTDETAILS_H

#include <QString>

class Product;

QString formatProductDetails(const Product &product, const QString &dataDirectory);

#endif
