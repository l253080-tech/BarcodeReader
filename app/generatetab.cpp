#include "generatetab.h"

#include <QLabel>
#include <QVBoxLayout>

GenerateTab::GenerateTab(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Generate tab (placeholder): 6-digit input, barcode, Save as PNG", this));
    layout->addStretch();
}
