// mainwindow.cpp
#include "mainwindow.h"
#include "barcodeutils.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QRegularExpressionValidator>
#include <QMediaDevices>
#include <QVideoFrame>
#include <QFileDialog>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    auto *tabs = new QTabWidget;
    setCentralWidget(tabs);

    // ---- Scan tab ----
    auto *scanPage = new QWidget;
    auto *scanLayout = new QVBoxLayout(scanPage);
    preview = new QVideoWidget;
    resultLabel = new QLabel("Point a barcode at the camera...");
    resultLabel->setAlignment(Qt::AlignCenter);
    scanLayout->addWidget(preview, 1);
    scanLayout->addWidget(resultLabel);
    tabs->addTab(scanPage, "Scan");

    camera = new QCamera(QMediaDevices::defaultVideoInput(), this);
    session.setCamera(camera);
    session.setVideoOutput(preview);
    connect(preview->videoSink(), &QVideoSink::videoFrameChanged,
            this, &MainWindow::onFrame);
    camera->start();

    // ---- Generate tab ----
    auto *genPage = new QWidget;
    auto *genLayout = new QVBoxLayout(genPage);
    codeEdit = new QLineEdit;
    codeEdit->setPlaceholderText("Enter exactly 6 digits");
    codeEdit->setMaxLength(6);
    codeEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression("^\\d{0,6}$"), this));
    barcodeView = new QLabel;
    barcodeView->setAlignment(Qt::AlignCenter);
    saveBtn = new QPushButton("Save as PNG");
    saveBtn->setEnabled(false);
    genLayout->addWidget(codeEdit);
    genLayout->addWidget(barcodeView, 1);
    genLayout->addWidget(saveBtn);
    tabs->addTab(genPage, "Generate");

    connect(codeEdit, &QLineEdit::textChanged, this, &MainWindow::onCodeChanged);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSave);
}

void MainWindow::onFrame(const QVideoFrame &frame)
{
    if (++frameCounter % 5 != 0) return;          // scan every 5th frame
    QImage img = frame.toImage();
    if (img.isNull()) return;

    QString text = decodeBarcode(img);
    if (text.isEmpty()) return;

    if (isValid6Digits(text))
        resultLabel->setText("Code: " + text);
    else
        resultLabel->setText("Invalid barcode (must be exactly 6 digits): " + text);
}

void MainWindow::onCodeChanged(const QString &text)
{
    bool ok = isValid6Digits(text);
    saveBtn->setEnabled(ok);
    if (ok) {
        currentBarcode = generateBarcode(text);
        barcodeView->setPixmap(QPixmap::fromImage(currentBarcode));
    } else {
        currentBarcode = QImage();
        barcodeView->clear();
    }
}

void MainWindow::onSave()
{
    QString path = QFileDialog::getSaveFileName(this, "Save barcode", "barcode.png", "PNG (*.png)");
    if (!path.isEmpty()) currentBarcode.save(path);
}
