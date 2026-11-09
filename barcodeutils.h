#pragma once
#include <QString>
#include <QImage>

bool isValid6Digits(const QString &s);
QImage generateBarcode(const QString &code6);   // 6-digit code -> barcode image
QString decodeBarcode(const QImage &img);       // barcode image -> text ("" if none found)