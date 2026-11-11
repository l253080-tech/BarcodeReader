
#include "mainwindow.h"

#include "generatetab.h"
#include "readtab.h"

#include <QTabWidget>
#include <QTabBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Barcode Reader");
    resize(900, 650);
    setMinimumSize(750, 550);

    // Create main tabs
    tabs = new QTabWidget(this);

    generateTab = new GenerateTab(tabs);
    readTab = new ReadTab(tabs);

    tabs->addTab(generateTab, "Generate Barcode");
    tabs->addTab(readTab, "Read Barcode");

    // Modern navigation tabs
    tabs->setDocumentMode(true);
    tabs->tabBar()->setExpanding(true);
    tabs->tabBar()->setUsesScrollButtons(false);

    setCentralWidget(tabs);

    // Application styling
    setStyleSheet(R"(

        QMainWindow {
            background-color: #F3F6FC;
        }

        QTabWidget::pane {
            background-color: #FFFFFF;
            border: 1px solid #DCE3EF;
            border-radius: 10px;
            top: -1px;
        }

        QTabBar {
            background-color: #E9EFF8;
        }

        QTabBar::tab {
            background-color: #E9EFF8;
            color: #52627A;
            min-width: 180px;
            min-height: 48px;
            padding: 8px 25px;
            margin-right: 5px;
            font-size: 15px;
            font-weight: bold;
            border: none;
            border-top-left-radius: 9px;
            border-top-right-radius: 9px;
        }

        QTabBar::tab:selected {
            background-color: #2563EB;
            color: white;
        }

        QTabBar::tab:hover:!selected {
            background-color: #D9E5F8;
            color: #1D4ED8;
        }

        QPushButton {
            background-color: #2563EB;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 22px;
            min-height: 38px;
            font-size: 14px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #1D4ED8;
        }

        QPushButton:pressed {
            background-color: #1E40AF;
        }

        QPushButton:disabled {
            background-color: #AAB8CC;
            color: #F4F6FA;
        }

        QPushButton#secondaryButton,
        QPushButton#saveButton {
            background-color: #E8EEF8;
            color: #1E40AF;
            border: 1px solid #C7D5EA;
        }

        QPushButton#secondaryButton:hover,
        QPushButton#saveButton:hover {
            background-color: #D7E5FA;
        }

        QLineEdit {
            background-color: white;
            color: #172554;
            border: 1px solid #CBD5E1;
            border-radius: 8px;
            padding: 10px;
            font-size: 15px;
        }

        QLineEdit:focus {
            border: 2px solid #3B82F6;
        }

        QLabel {
            color: #26364D;
        }

    )");
}
