// mainwindow.h
#pragma once
#include <QMainWindow>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void onFrame(const QVideoFrame &frame);
    void onCodeChanged(const QString &text);
    void onSave();
private:
    QCamera *camera;
    QMediaCaptureSession session;
    QVideoWidget *preview;
    QLabel *resultLabel;
    QLineEdit *codeEdit;
    QLabel *barcodeView;
    QPushButton *saveBtn;
    QImage currentBarcode;
    int frameCounter = 0;
};