#ifndef BILLDIALOG_H
#define BILLDIALOG_H

#include <QDialog>

#include "Bill.h"
#include "Order.h"

class ProductManager;
class QLabel;
class QTableWidget;

class BillDialog : public QDialog
{
    Q_OBJECT
public:
    BillDialog(const Bill &bill, const Order &order,
               const ProductManager &products, QWidget *parent = nullptr);

private slots:
    void printBill();

private:
    QString printableText() const;

    Bill bill_;
    Order order_;
    const ProductManager &products_;
    QTableWidget *itemsTable_ = nullptr;
    QLabel *summaryLabel_ = nullptr;
};

#endif
