#pragma once

#include <QImage>
#include <vector>

class ImageScanner
{
public:
    // Returns the width of every bar and space in modules (barcode = 68 modules),
    // starting with the first black bar. Empty vector = no barcode found.
    static std::vector<int> getWidths(const QImage &image);
};
