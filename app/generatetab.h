#ifndef GENERATETAB_H
#define GENERATETAB_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;

class GenerateTab : public QWidget
{
    Q_OBJECT

public:
    explicit GenerateTab(QWidget *parent = nullptr);

private:
    QLineEdit *numberInput;

    QPushButton *generateButton;
    QPushButton *saveButton;

    QLabel *barcodePreview;
    QLabel *barcodeNumber;
    QLabel *messageLabel;
};

#endif // GENERATETAB_H