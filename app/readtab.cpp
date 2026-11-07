#include "readtab.h"

#include <QLabel>
#include <QVBoxLayout>

ReadTab::ReadTab(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Read tab (placeholder): upload image, camera, decoded result", this));
    layout->addStretch();
}
