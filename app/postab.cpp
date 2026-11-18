#include "postab.h"

#include "BillingManager.h"
#include "billdialog.h"
#include "OrderManager.h"
#include "OrderFileHandler.h"
#include "ProductManager.h"
#include "productdetails.h"

#include <QComboBox>
#include <QAbstractItemView>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QStyle>
#include <QTableWidgetItem>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QFileInfo>

#include <cmath>

namespace {
QLabel *sectionTitle(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName("sectionTitle");
    return label;
}

QDoubleSpinBox *moneyInput(QWidget *parent)
{
    auto *input = new QDoubleSpinBox(parent);
    input->setRange(0.0, 100000000.0);
    input->setDecimals(2);
    input->setPrefix("PKR ");
    input->setMinimumHeight(38);
    return input;
}
}

PosTab::PosTab(ProductManager &products, OrderManager &orders,
               BillingManager &billing, OrderFileHandler &orderFiles,
               const QString &dataDirectory, QWidget *parent)
    : QWidget(parent), products_(products), orders_(orders), billing_(billing),
      orderFiles_(orderFiles), dataDirectory_(dataDirectory)
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *body = new QWidget;
    auto *root = new QVBoxLayout(body);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(16);
    outerLayout->addWidget(scrollArea);
    scrollArea->setWidget(body);

    auto *heading = new QLabel("Point of sale", body);
    heading->setObjectName("pageTitle");
    auto *subtitle = new QLabel("Scan products, build an order, and take payment.", body);
    subtitle->setObjectName("mutedText");
    root->addWidget(heading);
    root->addWidget(subtitle);

    auto *customerCard = new QWidget(body);
    customerCard->setObjectName("card");
    auto *customerForm = new QFormLayout(customerCard);
    customerForm->setContentsMargins(18, 16, 18, 16);
    customerForm->setHorizontalSpacing(20);
    customerForm->setVerticalSpacing(10);
    customerForm->addRow(sectionTitle("Customer", customerCard));
    customerName_ = new QLineEdit(customerCard);
    customerName_->setPlaceholderText("Walk-in customer");
    customerContact_ = new QLineEdit(customerCard);
    customerContact_->setPlaceholderText("Optional phone or contact");
    customerForm->addRow("Name", customerName_);
    customerForm->addRow("Contact", customerContact_);
    root->addWidget(customerCard);

    auto *productCard = new QWidget(body);
    productCard->setObjectName("card");
    auto *productLayout = new QVBoxLayout(productCard);
    productLayout->setContentsMargins(18, 16, 18, 16);
    productLayout->setSpacing(10);
    productLayout->addWidget(sectionTitle("Add products", productCard));
    auto *scanRow = new QHBoxLayout;
    barcodeInput_ = new QLineEdit(productCard);
    barcodeInput_->setPlaceholderText("Scan or enter a 6-digit barcode");
    barcodeInput_->setMaxLength(6);
    auto *lookupButton = new QPushButton("Find product", productCard);
    auto *scanButton = new QPushButton("Start camera scan", productCard);
    lookupButton->setMinimumHeight(40);
    scanButton->setMinimumHeight(40);
    scanButton->setObjectName("secondaryButton");
    scanRow->addWidget(barcodeInput_, 1);
    scanRow->addWidget(lookupButton);
    scanRow->addWidget(scanButton);
    productLayout->addLayout(scanRow);

    auto *productRow = new QHBoxLayout;
    productInfo_ = new QLabel("Enter a barcode or scan a product to see its details.", productCard);
    productInfo_->setObjectName("productInfo");
    productInfo_->setMinimumHeight(54);
    auto *descriptionButton = new QPushButton("Description", productCard);
    descriptionButton->setMinimumHeight(40);
    descriptionButton->setMinimumWidth(112);
    quantityInput_ = new QDoubleSpinBox(productCard);
    quantityInput_->setRange(1.0, 1000000.0);
    quantityInput_->setValue(1.0);
    quantityInput_->setDecimals(2);
    quantityInput_->setMinimumHeight(38);
    addButton_ = new QPushButton("Add to order", productCard);
    addButton_->setObjectName("primaryButton");
    addButton_->setEnabled(false);
    productRow->addWidget(productInfo_, 1);
    productRow->addWidget(descriptionButton);
    productRow->addWidget(new QLabel("Qty", productCard));
    productRow->addWidget(quantityInput_);
    productRow->addWidget(addButton_);
    productLayout->addLayout(productRow);
    root->addWidget(productCard);

    auto *cartCard = new QWidget(body);
    cartCard->setObjectName("card");
    auto *cartLayout = new QVBoxLayout(cartCard);
    cartLayout->setContentsMargins(18, 16, 18, 16);
    cartLayout->setSpacing(10);
    auto *cartHead = new QHBoxLayout;
    cartHead->addWidget(sectionTitle("Current order", cartCard));
    cartHead->addStretch();
    cartLayout->addLayout(cartHead);

    cartTable_ = new QTableWidget(0, 7, cartCard);
    cartTable_->setHorizontalHeaderLabels({"Product ID", "Product", "Unit price", "Quantity", "Tax", "Discount", "Line total"});
    cartTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    cartTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    cartTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    cartTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    cartTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartTable_->setAlternatingRowColors(true);
    cartTable_->setMinimumHeight(170);
    cartLayout->addWidget(cartTable_);
    connect(cartTable_, &QTableWidget::itemSelectionChanged, this, [this] {
        const int row = cartTable_->currentRow();
        if (row < 0 || !cartTable_->item(row, 0)) return;
        const auto product = products_.findById(cartTable_->item(row, 0)->text().toInt());
        if (!product) return;
        barcodeInput_->setText(QString::fromStdString(product->getBarcode()));
        lookupProduct();
        quantityInput_->setValue(cartTable_->item(row, 3)->text().toDouble());
    });

    auto *removeButton = new QPushButton("Remove selected", cartCard);
    auto *updateButton = new QPushButton("Update quantity", cartCard);
    auto *clearButton = new QPushButton("Clear order", cartCard);
    auto *holdButton = new QPushButton("Hold", cartCard);
    auto *resumeButton = new QPushButton("Resume held", cartCard);
    placeButton_ = new QPushButton("Place order", cartCard);
    placeButton_->setObjectName("primaryButton");
    auto *totals = new QHBoxLayout;
    subtotalLabel_ = new QLabel("Subtotal PKR 0.00", cartCard);
    taxLabel_ = new QLabel("Tax PKR 0.00", cartCard);
    discountLabel_ = new QLabel("Discount PKR 0.00", cartCard);
    payableLabel_ = new QLabel("Total payable PKR 0.00", cartCard);
    payableLabel_->setObjectName("totalLabel");
    totals->addWidget(subtotalLabel_);
    totals->addWidget(taxLabel_);
    totals->addWidget(discountLabel_);
    totals->addStretch();
    totals->addWidget(payableLabel_);
    cartLayout->addLayout(totals);
    for (QPushButton *button : {removeButton, updateButton, clearButton, holdButton, resumeButton, placeButton_})
        button->setMinimumHeight(40);
    auto *actionGrid = new QGridLayout;
    actionGrid->setHorizontalSpacing(8);
    actionGrid->setVerticalSpacing(8);
    actionGrid->addWidget(removeButton, 0, 0);
    actionGrid->addWidget(updateButton, 0, 1);
    actionGrid->addWidget(clearButton, 0, 2);
    actionGrid->addWidget(holdButton, 1, 0);
    actionGrid->addWidget(resumeButton, 1, 1);
    actionGrid->addWidget(placeButton_, 1, 2);
    for (int col = 0; col < 3; ++col) actionGrid->setColumnStretch(col, 1);
    cartLayout->addLayout(actionGrid);
    root->addWidget(cartCard);

    auto *paymentCard = new QWidget(body);
    paymentCard->setObjectName("card");
    auto *paymentForm = new QFormLayout(paymentCard);
    paymentForm->setContentsMargins(18, 14, 18, 14);
    paymentForm->addRow(sectionTitle("Payment", paymentCard));
    paymentInput_ = moneyInput(paymentCard);
    paymentMode_ = new QComboBox(paymentCard);
    paymentMode_->addItems({"Cash", "Card", "Wallet"});
    paymentMode_->setMinimumHeight(38);
    merchantName_ = new QLineEdit(paymentCard);
    merchantName_->setPlaceholderText("Required for card or wallet");
    paymentButton_ = new QPushButton("Complete payment", paymentCard);
    paymentButton_->setObjectName("primaryButton");
    paymentForm->addRow("Amount received", paymentInput_);
    paymentForm->addRow("Payment mode", paymentMode_);
    paymentForm->addRow("Merchant", merchantName_);
    paymentForm->addRow(QString(), paymentButton_);
    paymentButton_->setMinimumHeight(44);
    root->addWidget(paymentCard);

    statusLabel_ = new QLabel("Ready for a new order.", body);
    statusLabel_->setObjectName("mutedText");
    root->addWidget(statusLabel_);

    connect(lookupButton, &QPushButton::clicked, this, &PosTab::lookupProduct);
    connect(barcodeInput_, &QLineEdit::returnPressed, this, &PosTab::lookupProduct);
    connect(descriptionButton, &QPushButton::clicked, this, &PosTab::showProductDescription);
    connect(addButton_, &QPushButton::clicked, this, &PosTab::addToOrder);
    connect(scanButton, &QPushButton::clicked, this, [this] { emit cameraScanRequested(); });
    connect(placeButton_, &QPushButton::clicked, this, &PosTab::placeOrder);
    connect(holdButton, &QPushButton::clicked, this, &PosTab::holdOrder);
    connect(resumeButton, &QPushButton::clicked, this, &PosTab::resumeOrder);
    connect(paymentButton_, &QPushButton::clicked, this, &PosTab::completePayment);

    connect(removeButton, &QPushButton::clicked, this, [this] {
        const int row = cartTable_->currentRow();
        if (row < 0) { setStatus("Select a product row first.", true); return; }
        auto product = products_.findById(cartTable_->item(row, 0)->text().toInt());
        if (!product) { setStatus("Product is no longer available.", true); return; }
        const auto result = orders_.removeProductFromOrder(product->getBarcode());
        setStatus(QString::fromLocal8Bit(describe(result)), result != OrderResult::Success);
        refreshCart();
    });
    connect(updateButton, &QPushButton::clicked, this, [this] {
        const int row = cartTable_->currentRow();
        if (row < 0) { setStatus("Select a product row first.", true); return; }
        auto product = products_.findById(cartTable_->item(row, 0)->text().toInt());
        if (!product) { setStatus("Product is no longer available.", true); return; }
        const double quantity = quantityInput_->value();
        const auto result = orders_.updateProductQuantity(product->getBarcode(), quantity);
        setStatus(QString::fromLocal8Bit(describe(result)), result != OrderResult::Success);
        refreshCart();
    });
    connect(clearButton, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, "Clear order", "Discard the current order?") != QMessageBox::Yes)
            return;
        const Order *current = orders_.getCurrentOrder();
        const QString orderId = current ? QString::fromStdString(current->getOrderId()) : QString();
        const auto result = orders_.cancelOrder();
        if (result != OrderResult::Success && result != OrderResult::NoActiveOrder) {
            setStatus(QString::fromLocal8Bit(describe(result)), true); return;
        }
        if (!orderId.isEmpty()) {
            if (!orderFiles_.removeOrderDetails(orderId.toStdString()))
                QMessageBox::warning(this, "Could not clear order", "Could not write file:\n"
                    + QString::fromStdString(orderFiles_.orderDetailFilePath()));
            if (!orderFiles_.removeOrderRecord(orderId.toStdString()))
                QMessageBox::warning(this, "Could not clear order", "Could not write file:\n"
                    + QString::fromStdString(orderFiles_.orderFilePath()));
        }
        refreshCart();
        customerName_->clear();
        customerContact_->clear();
        setStatus("Current order cleared.");
    });
    connect(paymentMode_, &QComboBox::currentTextChanged, this, [this](const QString &selected) {
        merchantName_->setEnabled(selected != "Cash");
        if (selected == "Cash") merchantName_->clear();
    });
    merchantName_->setEnabled(false);
    refreshCart();
}

void PosTab::acceptBarcode(const QString &barcode)
{
    barcodeInput_->setText(barcode);
    lookupProduct();
    setStatus("Barcode scanned. Product lookup is ready.");
}

void PosTab::lookupProduct()
{
    const QString code = barcodeInput_->text().trimmed();
    auto product = products_.findByBarcode(code.toStdString());
    if (!product) {
        productInfo_->setText("No product found for barcode " + code + ".");
        addButton_->setEnabled(false);
        setStatus("Product not found. Check the barcode and try again.", true);
        return;
    }
    productInfo_->setText(QString("%1  ·  %2  ·  PKR %3  ·  Stock %4 %5")
        .arg(QString::fromStdString(product->getName()))
        .arg(QString::fromStdString(product->getBarcode()))
        .arg(product->getPrice(), 0, 'f', 2)
        .arg(product->getQuantity(), 0, 'f', 2)
        .arg(QString::fromStdString(product->getUnit())));
    const bool weighted = product->getUnit() == "weight";
    quantityInput_->setDecimals(weighted ? 1 : 0);
    quantityInput_->setSingleStep(weighted ? 0.1 : 1.0);
    if (weighted) {
        quantityInput_->setRange(0.1, std::max(0.1, product->getQuantity()));
        quantityInput_->setValue(0.1);
    } else {
        const int available = static_cast<int>(std::floor(product->getQuantity()));
        quantityInput_->setRange(1.0, std::max(1, available));
        quantityInput_->setValue(1.0);
    }
    addButton_->setEnabled(true);
    setStatus("Product found.");
}

void PosTab::showProductDescription()
{
    auto product = products_.findByBarcode(barcodeInput_->text().trimmed().toStdString());
    if (!product) { setStatus("Find a product first.", true); return; }
    QMessageBox::information(this, "Product details", formatProductDetails(*product, dataDirectory_));
}

void PosTab::addToOrder()
{
    const QString code = barcodeInput_->text().trimmed();
    auto product = products_.findByBarcode(code.toStdString());
    if (!product) { lookupProduct(); return; }
    const double quantity = quantityInput_->value();
    if (!(quantity > 0.0)) { setStatus("Enter a quantity greater than zero.", true); return; }
    const bool weighted = product->getUnit() == "weight";
    if (!weighted && std::fabs(quantity - std::round(quantity)) > 1e-9) {
        setStatus("Count products must use whole-number quantities.", true);
        return;
    }
    double alreadyInOrder = 0.0;
    if (const Order *current = orders_.getCurrentOrder())
        alreadyInOrder = current->getItemQuantity(product->getId());
    if (alreadyInOrder + quantity > product->getQuantity() + 1e-9) {
        setStatus(QString("Insufficient stock. Available: %1; already in cart: %2.")
            .arg(product->getQuantity(), 0, 'f', weighted ? 1 : 0)
            .arg(alreadyInOrder, 0, 'f', weighted ? 1 : 0), true);
        return;
    }
    if (!orders_.getCurrentOrder()) {
        const auto created = orders_.createOrder(customerName_->text().toStdString(), customerContact_->text().toStdString());
        if (created != OrderResult::Success) { setStatus(QString::fromLocal8Bit(describe(created)), true); return; }
    }
    const auto result = orders_.addProductToOrder(code.toStdString(), quantity);
    setStatus(QString::fromLocal8Bit(describe(result)), result != OrderResult::Success);
    refreshCart();
}

void PosTab::refreshCart()
{
    cartTable_->setRowCount(0);
    const Order *order = orders_.getCurrentOrder();
    if (!order) {
        subtotalLabel_->setText("Subtotal  PKR 0.00");
        taxLabel_->setText("Tax PKR 0.00");
        discountLabel_->setText("Discount PKR 0.00");
        payableLabel_->setText("Total payable PKR 0.00");
        paymentInput_->setValue(0.0);
        placeButton_->setEnabled(false);
        paymentButton_->setEnabled(false);
        return;
    }
    placeButton_->setEnabled(order->isEditable() && !order->getItems().empty());
    paymentButton_->setEnabled(order->isLocked());
    double subtotal = 0.0;
    double totalTax = 0.0;
    double totalDiscount = 0.0;
    for (const auto &item : order->getItems()) {
        auto product = products_.findByBarcode(item.getBarcode());
        if (!product) continue;
        const double lineSubtotal = product->getPrice() * item.getQuantity();
        const double tax = product->getTax() * item.getQuantity();
        const double discount = product->getDiscount() * item.getQuantity();
        const int row = cartTable_->rowCount();
        cartTable_->insertRow(row);
        const QString values[] = {
            QString::number(product->getId()), QString::fromStdString(product->getName()),
            QString("PKR %1").arg(product->getPrice(), 0, 'f', 2),
            QString::number(item.getQuantity(), 'f', product->getUnit() == "weight" ? 1 : 0),
            QString("PKR %1").arg(tax, 0, 'f', 2),
            QString("PKR %1").arg(discount, 0, 'f', 2),
            QString("PKR %1").arg(std::max(0.0, lineSubtotal + tax - discount), 0, 'f', 2)
        };
        for (int column = 0; column < 7; ++column) cartTable_->setItem(row, column, new QTableWidgetItem(values[column]));
        subtotal += lineSubtotal;
        totalTax += tax;
        totalDiscount += discount;
    }
    const double payable = std::max(0.0, subtotal + totalTax - totalDiscount);
    subtotalLabel_->setText(QString("Subtotal PKR %1").arg(subtotal, 0, 'f', 2));
    taxLabel_->setText(QString("Tax PKR %1").arg(totalTax, 0, 'f', 2));
    discountLabel_->setText(QString("Discount PKR %1").arg(totalDiscount, 0, 'f', 2));
    payableLabel_->setText(QString("Total payable PKR %1").arg(payable, 0, 'f', 2));
    paymentInput_->setValue(payable);
}

void PosTab::placeOrder()
{
    const Order *order = orders_.getCurrentOrder();
    if (order && order->isEditable()) {
        if (QMessageBox::question(this, "Place order", "Lock this order and continue to payment?") != QMessageBox::Yes)
            return;
        const auto result = orders_.placeOrder();
        setStatus(QString::fromLocal8Bit(describe(result)), result != OrderResult::Success);
        if (result == OrderResult::Success) {
            const Order *placed = orders_.getCurrentOrder();
            if (placed) persistOrder(*placed);
        }
        refreshCart();
    }
}

void PosTab::holdOrder()
{
    const Order *active = orders_.getCurrentOrder();
    const QString orderId = active ? QString::fromStdString(active->getOrderId()) : QString();
    const auto result = orders_.holdOrder();
    setStatus(QString::fromLocal8Bit(describe(result)), result != OrderResult::Success);
    if (result == OrderResult::Success) {
        const auto held = orders_.getHeldOrders().find(orderId.toStdString());
        if (held != orders_.getHeldOrders().end()) persistOrder(held->second);
    }
    refreshCart();
}

void PosTab::resumeOrder()
{
    QStringList ids;
    for (const auto &entry : orders_.getHeldOrders()) ids << QString::fromStdString(entry.first);
    if (ids.empty()) { setStatus("There are no held orders.", true); return; }
    bool ok = false;
    const QString selected = QInputDialog::getItem(this, "Resume order", "Held order", ids, 0, false, &ok);
    if (!ok) return;
    const auto result = orders_.resumeOrder(selected.toStdString());
    if (result == OrderResult::Success) {
        const Order *order = orders_.getCurrentOrder();
        if (order) {
            customerName_->setText(QString::fromStdString(order->getCustomerName()));
            customerContact_->setText(QString::fromStdString(order->getCustomerContact()));
            persistOrder(*order);
        }
    }
    setStatus(QString::fromLocal8Bit(describe(result)), result != OrderResult::Success);
    refreshCart();
}

void PosTab::completePayment()
{
    const Order *order = orders_.getCurrentOrder();
    if (!order || !order->isLocked()) { setStatus("Place the order before payment.", true); return; }
    if (order->getItems().empty()) { setStatus("Cannot pay for an empty order.", true); return; }
    const double payable = billing_.calculateTotalPayable(*order);
    if (paymentInput_->value() + 1e-6 < payable) {
        setStatus(QString("Payment is short. Amount due: PKR %1.").arg(payable, 0, 'f', 2), true);
        return;
    }
    PaymentMode paymentMode = PaymentMode::Cash;
    if (paymentMode_->currentText() == "Card") paymentMode = PaymentMode::Card;
    else if (paymentMode_->currentText() == "Wallet") paymentMode = PaymentMode::Wallet;
    if (paymentMode != PaymentMode::Cash && merchantName_->text().trimmed().isEmpty()) {
        setStatus("Enter the card or wallet merchant name.", true);
        return;
    }
    const Order orderSnapshot = *order;
    billing_.setPaymentDetails(paymentInput_->value(), paymentMode, merchantName_->text().toStdString());
    std::optional<Bill> bill;
    const BillingResult result = billing_.finalizeBill(bill);
    if (result != BillingResult::Success || !bill) {
        const QString path = QString::fromStdString(billing_.getLastPersistenceErrorPath());
        if (!path.isEmpty()) {
            QMessageBox::warning(this, "Persistence failed", QString::fromLocal8Bit(describe(result))
                + "\nCould not write file:\n" + path);
            setStatus(QString::fromLocal8Bit(describe(result)) + " See the file path in the warning.");
            return;
        }
        if (result == BillingResult::InventoryUpdateFailed) {
            QMessageBox::warning(this, "Inventory update failed", QString::fromLocal8Bit(describe(result))
                + "\nCould not write file:\n" + QString::fromStdString(products_.getProductFilePath()));
            setStatus(QString::fromLocal8Bit(describe(result)) + " See the file path in the warning.");
            return;
        }
        setStatus(QString::fromLocal8Bit(describe(result)), true);
        return;
    }
    BillDialog dialog(*bill, orderSnapshot, products_, this);
    dialog.exec();
    products_.loadProducts();
    paymentInput_->setValue(0.0);
    merchantName_->clear();
    paymentMode_->setCurrentIndex(0);
    quantityInput_->setRange(1.0, 1000000.0);
    quantityInput_->setDecimals(0);
    quantityInput_->setSingleStep(1.0);
    quantityInput_->setValue(1.0);
    customerName_->clear();
    customerContact_->clear();
    barcodeInput_->clear();
    productInfo_->setText("Enter a barcode or scan a product to see its details.");
    addButton_->setEnabled(false);
    refreshCart();
    setStatus("Payment complete. Bill saved successfully.");
}

void PosTab::setStatus(const QString &message, bool error)
{
    statusLabel_->setText(message);
    statusLabel_->setProperty("error", error);
    statusLabel_->style()->unpolish(statusLabel_);
    statusLabel_->style()->polish(statusLabel_);
    if (error)
        QMessageBox::warning(this, "POS", message);
}

bool PosTab::persistOrder(const Order &order)
{
    if (!orderFiles_.saveOrder(order)) {
        QMessageBox::warning(this, "Order save failed", "Could not write file:\n"
            + QString::fromStdString(orderFiles_.orderFilePath()));
        return false;
    }
    if (!orderFiles_.saveOrderDetails(order)) {
        QMessageBox::warning(this, "Order items save failed", "Could not write file:\n"
            + QString::fromStdString(orderFiles_.orderDetailFilePath()));
        return false;
    }
    return true;
}
