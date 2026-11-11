#ifndef READTAB_H
#define READTAB_H

#include <QWidget>
#include <QString>
class QLabel;
class QPushButton;

class ReadTab : public QWidget
{
    Q_OBJECT

public:
    explicit ReadTab(QWidget *parent = nullptr);

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
};

#endif // READTAB_H