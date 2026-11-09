#include "barcodeutils.h"
#include <QRegularExpression>
#include "ReadBarcode.h"
#include "BarcodeFormat.h"
#include "MultiFormatWriter.h"
#include "BitMatrix.h"

bool isValid6Digits(const QString &s)
{
    static const QRegularExpression re("^\\d{6}$");
    return re.match(s).hasMatch();
}

QImage generateBarcode(const QString &code6)
{
    if (!isValid6Digits(code6))
        return QImage();

    ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::Code128);
    writer.setMargin(10);
    ZXing::BitMatrix matrix = writer.encode(code6.toStdString(), 400, 120);

    QImage img(matrix.width(), matrix.height(), QImage::Format_Grayscale8);
    for (int y = 0; y < matrix.height(); ++y)
        for (int x = 0; x < matrix.width(); ++x)
            img.setPixel(x, y, matrix.get(x, y) ? 0 : 255);
    return img;
}

QString decodeBarcode(const QImage &src)
{
    QImage gray = src.convertToFormat(QImage::Format_Grayscale8);
    ZXing::ImageView view(gray.constBits(), gray.width(), gray.height(),
                          ZXing::ImageFormat::Lum, gray.bytesPerLine());

    ZXing::ReaderOptions opts;
    opts.setFormats(ZXing::BarcodeFormat::Code128);

    ZXing::Result result = ZXing::ReadBarcode(view, opts);
    if (!result.isValid())
        return QString();
    return QString::fromStdString(result.text());
}