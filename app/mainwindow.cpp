#include "mainwindow.h"

#include "generatetab.h"
#include "readtab.h"

#include <QTabWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Barcode Reader");
    resize(800, 600);

    tabs = new QTabWidget(this);
    generateTab = new GenerateTab(tabs);
    readTab = new ReadTab(tabs);

    tabs->addTab(generateTab, "Generate");
    tabs->addTab(readTab, "Read");

    setCentralWidget(tabs);
}
