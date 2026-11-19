#include "mainwindow.h"

#include "generatetab.h"
#include "readtab.h"
#include "postab.h"
#include "ProductFileHandler.h"
#include "ProductManager.h"
#include "ProductManagerCatalog.h"
#include "OrderManager.h"
#include "BillingManager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QTabBar>
#include <QStyle>
#include <QFile>
#include <QMessageBox>
#include <QStringList>

#include <fstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Counterpoint | Retail POS");
    setMinimumSize(980, 640);
    resize(1380, 900);

    const QString dataDirectory = QCoreApplication::applicationDirPath();
    QDir dataDir(dataDirectory);
    dataDir.mkpath("barcodes");
    bool transactionFilesReady = true;
    QStringList failedStartupPaths;
    const QStringList requiredFiles{"Product.txt", "Order.txt", "OrderDetail.txt", "Bill.txt"};
    for (const QString &fileName : requiredFiles) {
        QFile file(dataDir.filePath(fileName));
        if (!file.exists() && !file.open(QIODevice::WriteOnly)) {
            transactionFilesReady = false;
            failedStartupPaths << QFileInfo(file).absoluteFilePath();
        }
        if (file.isOpen())
            file.close();
    }
    const QString dataPath = dataDir.filePath("Product.txt");
    productFileHandler_ = std::make_unique<ProductFileHandler>(QFileInfo(dataPath).absoluteFilePath().toStdString());
    productManager_ = std::make_unique<ProductManager>(*productFileHandler_);
    const bool productFileOpened = productManager_->loadProducts();
    std::ifstream productInput(dataPath.toStdString());
    size_t nonEmptyLines = 0;
    std::string productLine;
    while (std::getline(productInput, productLine))
        if (productLine.find_first_not_of(" \t\r\n") != std::string::npos) ++nonEmptyLines;
    const bool productsLoaded = productFileOpened && nonEmptyLines > 0
                                && nonEmptyLines == productManager_->getAllProducts().size();
    productCatalog_ = std::make_unique<ProductManagerCatalog>(*productManager_);
    orderManager_ = std::make_unique<OrderManager>(*productCatalog_);
    orderFileHandler_ = std::make_unique<OrderFileHandler>(
        dataDir.filePath("Order.txt").toStdString(),
        dataDir.filePath("OrderDetail.txt").toStdString());
    billFileHandler_ = std::make_unique<BillFileHandler>(dataDir.filePath("Bill.txt").toStdString());
    billingManager_ = std::make_unique<BillingManager>(
        *productManager_, *orderManager_, *orderFileHandler_, *billFileHandler_);

    auto *shell = new QWidget(this);
    auto *shellLayout = new QHBoxLayout(shell);
    shellLayout->setContentsMargins(0, 0, 0, 0);
    shellLayout->setSpacing(0);

    auto *sidebar = new QWidget(shell);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(228);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(18, 24, 18, 18);
    sideLayout->setSpacing(8);
    auto *brand = new QLabel("COUNTERPOINT", sidebar);
    brand->setObjectName("brand");
    auto *brandSub = new QLabel("RETAIL POS", sidebar);
    brandSub->setObjectName("brandSub");
    sideLayout->addWidget(brand);
    sideLayout->addWidget(brandSub);
    sideLayout->addSpacing(30);
    auto *navCaption = new QLabel("WORKSPACE", sidebar);
    navCaption->setObjectName("navCaption");
    sideLayout->addWidget(navCaption);

    auto *content = new QWidget(shell);
    content->setObjectName("contentArea");
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    auto *topbar = new QWidget(content);
    topbar->setObjectName("topbar");
    auto *topLayout = new QHBoxLayout(topbar);
    topLayout->setContentsMargins(28, 12, 28, 12);
    auto *topTitle = new QLabel("Store operations", topbar);
    topTitle->setObjectName("topTitle");
    const bool storageReady = productsLoaded && transactionFilesReady;
    auto *storeStatus = new QLabel(storageReady ? QString("●  Inventory loaded  ·  data: ") + QFileInfo(dataDirectory).absoluteFilePath() : QString("●  Inventory / data file issue"), topbar);
    storeStatus->setObjectName(storageReady ? "storeStatus" : "storeStatusError");
    storeStatus->setToolTip(QFileInfo(dataDirectory).absoluteFilePath());
    topLayout->addWidget(topTitle);
    topLayout->addStretch();
    topLayout->addWidget(storeStatus);

    tabs = new QTabWidget(content);
    tabs->tabBar()->hide();
    generateTab = new GenerateTab(tabs);
    readTab = new ReadTab(*productManager_, dataDirectory, tabs);
    posTab = new PosTab(*productManager_, *orderManager_, *billingManager_,
                        *orderFileHandler_, dataDirectory, tabs);

    tabs->addTab(posTab, "POS");
    tabs->addTab(generateTab, "Generate");
    tabs->addTab(readTab, "Read");
    contentLayout->addWidget(topbar);
    contentLayout->addWidget(tabs, 1);

    QTabWidget *tabWidget = tabs;
    auto addNavButton = [sideLayout, tabWidget, sidebar](const QString &text, int index, bool primary = false) {
        auto *button = new QPushButton(text, sidebar);
        button->setObjectName(primary ? "navButtonActive" : "navButton");
        button->setCheckable(true);
        button->setChecked(primary);
        sideLayout->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, tabWidget, [button, tabWidget, sidebar, index] {
            tabWidget->setCurrentIndex(index);
            for (auto *candidate : sidebar->findChildren<QPushButton *>()) {
                candidate->setChecked(candidate == button);
                candidate->setObjectName(candidate == button ? "navButtonActive" : "navButton");
                candidate->style()->unpolish(candidate);
                candidate->style()->polish(candidate);
            }
        });
        return button;
    };
    addNavButton("▦   Point of sale", 0, true);
    addNavButton("▤   Barcode generator", 1);
    addNavButton("⌕   Barcode reader", 2);
    sideLayout->addStretch();
    shellLayout->addWidget(sidebar);
    shellLayout->addWidget(content, 1);

    connect(posTab, &PosTab::cameraScanRequested, this, [this] {
        readTab->startCameraScan(true);
    });
    connect(readTab, &ReadTab::barcodeDecoded, posTab, &PosTab::acceptBarcode);

    setCentralWidget(shell);
    setStyleSheet(R"(
        QWidget { font-family: "Segoe UI"; font-size: 10pt; color: #1e293b; }
        QMainWindow { background: #f3f6fb; }
        #sidebar { background: #132238; }
        #brand { color: white; font-size: 18pt; font-weight: 800; letter-spacing: 1px; }
        #brandSub, #navCaption { color: #8293aa; font-size: 8pt; font-weight: 700; letter-spacing: 1.2px; }
        #navCaption { margin-top: 8px; margin-bottom: 4px; }
        QPushButton#navButton, QPushButton#navButtonActive { text-align: left; color: #c1ccdb; background: transparent; border: 0; border-radius: 9px; padding: 12px 13px; font-weight: 600; }
        QPushButton#navButton:hover { background: #233952; color: white; }
        QPushButton#navButtonActive { background: #2c5e9e; color: white; }
        #sidebarFoot { color: #8293aa; font-size: 8pt; line-height: 1.5; padding: 10px 4px; }
        #contentArea { background: #f3f6fb; }
        #topbar { background: white; border-bottom: 1px solid #e5eaf1; }
        #topTitle { font-size: 11pt; font-weight: 650; }
        #storeStatus { color: #16835d; font-size: 9pt; font-weight: 600; }
        #storeStatusError { color: #bd4b4b; font-size: 9pt; font-weight: 600; }
        #pageTitle { font-size: 23pt; font-weight: 750; color: #14243a; }
        #mutedText { color: #738198; }
        QLabel[error="true"] { color: #b33d45; }
        #card { background: white; border: 1px solid #e5eaf1; border-radius: 12px; }
        #sectionTitle { font-size: 11pt; font-weight: 700; color: #1b2c43; }
        #productInfo { color: #35465c; background: #f6f8fb; border-radius: 7px; padding: 8px 10px; }
        #totalLabel { color: #14243a; font-weight: 750; font-size: 11pt; }
        QLineEdit, QComboBox, QDoubleSpinBox { background: white; border: 1px solid #d7dfeb; border-radius: 7px; padding: 7px 10px; selection-background-color: #3978bc; }
        QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 1px solid #3978bc; }
        QPushButton { background: #eaf0f7; color: #253951; border: 1px solid #dce4ee; border-radius: 7px; padding: 8px 12px; font-weight: 600; }
        QPushButton:hover { background: #dce8f5; }
        QPushButton#primaryButton { background: #2766a8; color: white; border: 1px solid #2766a8; }
        QPushButton#primaryButton:hover { background: #1e568f; }
        QPushButton#secondaryButton { background: #e9f3ff; color: #24609d; border: 1px solid #d4e6f8; }
        QTableWidget { background: white; alternate-background-color: #f8fafc; border: 1px solid #e5eaf1; border-radius: 7px; gridline-color: #edf0f5; }
        QHeaderView::section { background: #f2f5f9; color: #52627a; border: 0; border-bottom: 1px solid #e4e9f0; padding: 9px 7px; font-weight: 700; }
        QTableWidget::item { padding: 5px; }
        QTabWidget::pane { border: 0; }
        QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
        QScrollBar::handle:vertical { background: #c4cedb; border-radius: 5px; min-height: 25px; }
    )");

    if (!productsLoaded) {
        const QString reason = productFileOpened
                                   ? "Product.txt is empty or contains malformed product records."
                                   : "Product.txt could not be opened.";
        QMessageBox::warning(this, "Inventory unavailable", reason + "\nFile:\n" + QFileInfo(dataPath).absoluteFilePath());
    } else if (!transactionFilesReady) {
        QMessageBox::warning(this, "Data files unavailable",
                             "Could not create or open these data files:\n" + failedStartupPaths.join("\n"));
    }
}

MainWindow::~MainWindow() = default;