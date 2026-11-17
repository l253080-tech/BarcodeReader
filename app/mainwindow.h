#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class QTabWidget;
class GenerateTab;
class ReadTab;
class PosTab;
class ProductFileHandler;
class ProductManager;
class ProductManagerCatalog;
class OrderManager;
class BillingManager;
class OrderFileHandler;
class BillFileHandler;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    QTabWidget *tabs;
    GenerateTab *generateTab;
    ReadTab *readTab;
    PosTab *posTab;
    std::unique_ptr<ProductFileHandler> productFileHandler_;
    std::unique_ptr<ProductManager> productManager_;
    std::unique_ptr<ProductManagerCatalog> productCatalog_;
    std::unique_ptr<OrderManager> orderManager_;
    std::unique_ptr<OrderFileHandler> orderFileHandler_;
    std::unique_ptr<BillFileHandler> billFileHandler_;
    std::unique_ptr<BillingManager> billingManager_;
};

#endif // MAINWINDOW_H
