#ifndef POSTAB_H
#define POSTAB_H

#include <QWidget>
#include <QString>

class BillingManager;
class ProductManager;
class OrderManager;
class OrderFileHandler;
class Order;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QDoubleSpinBox;
class QComboBox;

class PosTab : public QWidget
{
    Q_OBJECT
public:
    PosTab(ProductManager &products, OrderManager &orders,
           BillingManager &billing, OrderFileHandler &orderFiles,
           const QString &dataDirectory,
           QWidget *parent = nullptr);

signals:
    void cameraScanRequested();

public slots:
    void acceptBarcode(const QString &barcode);

private:
    void lookupProduct();
    void refreshCart();
    void showProductDescription();
    void addToOrder();
    void placeOrder();
    void holdOrder();
    void resumeOrder();
    void completePayment();
    void setStatus(const QString &message, bool error = false);
    bool persistOrder(const Order &order);

    ProductManager &products_;
    OrderManager &orders_;
    BillingManager &billing_;
    OrderFileHandler &orderFiles_;
    QString dataDirectory_;
    QLineEdit *barcodeInput_ = nullptr;
    QLineEdit *customerName_ = nullptr;
    QLineEdit *customerContact_ = nullptr;
    QLineEdit *merchantName_ = nullptr;
    QDoubleSpinBox *quantityInput_ = nullptr;
    QDoubleSpinBox *paymentInput_ = nullptr;
    QComboBox *paymentMode_ = nullptr;
    QTableWidget *cartTable_ = nullptr;
    QLabel *productInfo_ = nullptr;
    QLabel *subtotalLabel_ = nullptr;
    QLabel *taxLabel_ = nullptr;
    QLabel *discountLabel_ = nullptr;
    QLabel *payableLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QPushButton *addButton_ = nullptr;
    QPushButton *placeButton_ = nullptr;
    QPushButton *paymentButton_ = nullptr;
};

#endif
