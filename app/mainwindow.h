#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTabWidget;
class GenerateTab;
class ReadTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QTabWidget *tabs;
    GenerateTab *generateTab;
    ReadTab *readTab;
};

#endif // MAINWINDOW_H
