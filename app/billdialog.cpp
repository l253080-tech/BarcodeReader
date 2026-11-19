#include "billdialog.h"

#include "ProductManager.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QPrinter>
#include <QPrintDialog>
#include <algorithm>

BillDialog::BillDialog(const Bill &bill, const Order &order,
                       const ProductManager &products, QWidget *parent)
    : QDialog(parent), bill_(bill), order_(order), products_(products)
{
    setWindowTitle("Payment complete · Bill " + QString::fromStdString(bill_.getBillId()));
    resize(760, 620);

    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("Sale completed", this);
    title->setStyleSheet("font-size: 20pt; font-weight: 700; color: #16304b;");
    layout->addWidget(title);

    auto *metadata = new QLabel(
        QString("<b>Bill ID</b> %1 &nbsp;&nbsp; <b>Order ID</b> %2<br>"
                "<b>Customer</b> %3<br><b>Date</b> %4 &nbsp;&nbsp; <b>Time</b> %5")
            .arg(QString::fromStdString(bill_.getBillId()))
            .arg(QString::fromStdString(order_.getOrderId()))
            .arg(QString::fromStdString(bill_.getCustomerName().empty() ? "Walk-in customer" : bill_.getCustomerName()))
            .arg(QString::fromStdString(order_.getOrderDate()))
            .arg(QString::fromStdString(order_.getOrderTime())), this);
    metadata->setTextFormat(Qt::RichText);
    layout->addWidget(metadata);

    itemsTable_ = new QTableWidget(0, 6, this);
    itemsTable_->setHorizontalHeaderLabels({"Product", "Unit price", "Discount / unit", "Quantity", "Tax", "Line total"});
    itemsTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    itemsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    itemsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (const auto &item : bill_.getItems()) {
        const auto product = products_.findByBarcode(item.barcode);
        const QString name = product
                                 ? QString::fromStdString(product->getAcronym() + " · " + product->getName())
                                 : QString::fromStdString(item.productAcronym);
        const int row = itemsTable_->rowCount();
        itemsTable_->insertRow(row);
        const QString cells[] = {
            name,
            QString("PKR %1").arg(item.unitPriceWithoutTax, 0, 'f', 2),
            QString("PKR %1").arg(item.discountPerUnit, 0, 'f', 2),
            QString::number(item.quantity, 'f', 2),
            QString("PKR %1").arg(item.getLineTax(), 0, 'f', 2),
            QString("PKR %1").arg(std::max(0.0, item.getLinePayable()), 0, 'f', 2)
        };
        for (int col = 0; col < 6; ++col)
            itemsTable_->setItem(row, col, new QTableWidgetItem(cells[col]));
    }
    layout->addWidget(itemsTable_, 1);

    QString totalsText = QString("Subtotal: PKR %1<br>Total tax: PKR %2<br>Total discount: PKR %3<br>"
                                 "<b>Total payable: PKR %4</b><br>Payment status: %5")
                             .arg(bill_.getTotalWithoutTax(), 0, 'f', 2)
                             .arg(bill_.getTotalTax(), 0, 'f', 2)
                             .arg(bill_.getTotalDiscount(), 0, 'f', 2)
                             .arg(bill_.getTotalPayable(), 0, 'f', 2)
                             .arg(QString::fromStdString(bill_.getPaymentStatus()));
    if (order_.getPaymentMode() == PaymentMode::Cash)
        totalsText += QString("<br>Change due: PKR %1")
                          .arg(std::max(0.0, order_.getPaymentAmount() - bill_.getTotalPayable()), 0, 'f', 2);
    if (order_.getPaymentMode() != PaymentMode::NotSet) {
        totalsText += QString("<br>Paid by: %1").arg(QString::fromStdString(toString(order_.getPaymentMode())));
        if (!order_.getMerchantName().empty())
            totalsText += " (" + QString::fromStdString(order_.getMerchantName()).toHtmlEscaped() + ")";
    }
    summaryLabel_ = new QLabel(
        totalsText, this);
    summaryLabel_->setTextFormat(Qt::RichText);
    layout->addWidget(summaryLabel_);

    auto *buttons = new QDialogButtonBox(this);
    auto *printButton = buttons->addButton("Print", QDialogButtonBox::ActionRole);
    auto *closeButton = buttons->addButton(QDialogButtonBox::Close);
    layout->addWidget(buttons);
    connect(printButton, &QPushButton::clicked, this, &BillDialog::printBill);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

QString BillDialog::printableText() const
{
    QString text = QString("<h1>Counterpoint · Retail POS</h1><p>Bill %1 · Order %2<br>Customer: %3<br>Date: %4 %5</p><table border='1' cellspacing='0' cellpadding='5'><tr><th>Product</th><th>Unit price</th><th>Discount</th><th>Qty</th><th>Tax</th><th>Line total</th></tr>")
                       .arg(QString::fromStdString(bill_.getBillId()))
                       .arg(QString::fromStdString(order_.getOrderId()))
                       .arg(QString::fromStdString(bill_.getCustomerName()))
                       .arg(QString::fromStdString(order_.getOrderDate()))
                       .arg(QString::fromStdString(order_.getOrderTime()));
    for (const auto &item : bill_.getItems()) {
        const auto product = products_.findByBarcode(item.barcode);
        const QString name = product ? QString::fromStdString(product->getName())
                                     : QString::fromStdString(item.productAcronym);
        text += QString("<tr><td>%1</td><td>PKR %2</td><td>PKR %3</td><td>%4</td><td>PKR %5</td><td>PKR %6</td></tr>")
                    .arg(name.toHtmlEscaped())
                    .arg(item.unitPriceWithoutTax, 0, 'f', 2)
                    .arg(item.discountPerUnit, 0, 'f', 2)
                    .arg(item.quantity, 0, 'f', 2)
                    .arg(item.getLineTax(), 0, 'f', 2)
                    .arg(std::max(0.0, item.getLinePayable()), 0, 'f', 2);
    }
    text += QString("</table><p>Subtotal: PKR %1<br>Tax: PKR %2<br>Discount: PKR %3<br><b>Total payable: PKR %4</b><br>Payment status: %5</p>")
                .arg(bill_.getTotalWithoutTax(), 0, 'f', 2)
                .arg(bill_.getTotalTax(), 0, 'f', 2)
                .arg(bill_.getTotalDiscount(), 0, 'f', 2)
                .arg(bill_.getTotalPayable(), 0, 'f', 2)
                .arg(QString::fromStdString(bill_.getPaymentStatus()));
    if (order_.getPaymentMode() == PaymentMode::Cash)
        text += QString("<p>Change due: PKR %1</p>")
                    .arg(std::max(0.0, order_.getPaymentAmount() - bill_.getTotalPayable()), 0, 'f', 2);
    if (order_.getPaymentMode() != PaymentMode::NotSet) {
        text += QString("<p>Paid by: %1").arg(QString::fromStdString(toString(order_.getPaymentMode())));
        if (!order_.getMerchantName().empty())
            text += " (" + QString::fromStdString(order_.getMerchantName()).toHtmlEscaped() + ")";
        text += "</p>";
    }
    return text;
}

void BillDialog::printBill()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    QTextDocument document;
    document.setHtml(printableText());
    document.print(&printer);
}