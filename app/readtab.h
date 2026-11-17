#ifndef READTAB_H
#define READTAB_H

#include <QWidget>
#include <QString>
class QLabel;
class QPushButton;
class ProductManager;

class ReadTab : public QWidget
{
    Q_OBJECT

public:
    ReadTab(ProductManager &products, const QString &dataDirectory, QWidget *parent = nullptr);

public slots:
    void startCameraScan(bool directToPos = false);

signals:
    void barcodeDecoded(const QString &barcode);

private:
    QPushButton *uploadButton;
    QPushButton *cameraButton;
    QPushButton *decodeButton;

    QLabel *imagePreview;
    QLabel *fileNameLabel;
    QLabel *resultHeading;
    QLabel *resultLabel;
    QLabel *messageLabel;

    QString imagePath;
    ProductManager &products_;
    QString dataDirectory_;
    bool directToPos_ = false;
    void decodeCurrentImage();
};

#endif // READTAB_H
