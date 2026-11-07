#ifndef GENERATETAB_H
#define GENERATETAB_H

#include <QWidget>

// Generate tab: 6-digit input, barcode drawing, Save as PNG.
class GenerateTab : public QWidget
{
    Q_OBJECT

public:
    explicit GenerateTab(QWidget *parent = nullptr);
};

#endif // GENERATETAB_H
