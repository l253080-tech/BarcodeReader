#ifndef READTAB_H
#define READTAB_H

#include <QWidget>

// Read tab: upload an image or use the camera, then show the decoded 6 digits.
class ReadTab : public QWidget
{
    Q_OBJECT

public:
    explicit ReadTab(QWidget *parent = nullptr);
};

#endif // READTAB_H
