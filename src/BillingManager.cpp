#include "BillingManager.h"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std;

namespace {
tm toLocal(time_t t) {
    tm tmStruct{};
#ifdef _WIN32
    localtime_s(&tmStruct, &t);
#else
    localtime_r(&t, &tmStruct);
#endif
    return tmStruct;
}
} // namespace

BillingManager::BillingManager(ProductManager& productManager,
                               OrderManager& orderManager,
                               OrderFileHandler orderFileHandler,
                               BillFileHandler billFileHandler,
                               Clock clock)
    : productManager_(productManager),
      orderManager_(orderManager),
      orderFileHandler_(move(orderFileHandler)),
      billFileHandler_(move(billFileHandler)),
      clock_(move(clock)) {
    for (const auto& existingBill : billFileHandler_.loadBills()) {
        registerExistingBillId(existingBill.getBillId());
    }
    restoreSavedOrders();
}

void BillingManager::restoreSavedOrders() {
    for (const auto& existingOrder : orderFileHandler_.loadOrders()) {
        orderManager_.registerExistingOrderId(existingOrder.getOrderId());
        if (existingOrder.getOrderStatus() == OrderStatus::OnHold) {
            orderManager_.restoreHeldOrder(existingOrder);
        }
    }
}

void BillingManager::registerExistingBillId(const string& billId) {
    if (billId.size() != 9) {
        return;
    }
    for (char c : billId) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            return;
        }
    }
    usedBillIds_.insert(billId);
}

string BillingManager::generateBillId() {
    tm tmStruct = toLocal(clock_());
    int dayOfMonth = tmStruct.tm_mday;
    if (dayOfMonth < 1 || dayOfMonth > 31) {
        dayOfMonth = 1;
    }

    for (int seq = 1; seq <= 9999999; ++seq) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%07d%02d", seq, dayOfMonth);
        if (usedBillIds_.insert(buf).second) {
            return buf;
        }
    }
    throw runtime_error("No free 9-digit Bill IDs left.");
}

double BillingManager::calculateSubtotal(const Order& order) const {
    double subtotal = 0.0;
    for (const auto& item : order.getItems()) {
        auto prod = productManager_.findByBarcode(item.getBarcode());
        if (prod) {
            subtotal += prod->getPrice() * item.getQuantity();
        }
    }
    return subtotal;
}

double BillingManager::calculateTotalTax(const Order& order) const {
    double totalTax = 0.0;
    for (const auto& item : order.getItems()) {
        auto prod = productManager_.findByBarcode(item.getBarcode());
        if (prod) {
            totalTax += prod->getTax() * item.getQuantity();
        }
    }
    return totalTax;
}

double BillingManager::calculateTotalDiscount(const Order& order) const {
    double totalDiscount = 0.0;
    for (const auto& item : order.getItems()) {
        auto prod = productManager_.findByBarcode(item.getBarcode());
        if (prod) {
            totalDiscount += prod->getDiscount() * item.getQuantity();
        }
    }
    return totalDiscount;
}

double BillingManager::calculateTotalPayable(const Order& order) const {
    double payable = calculateSubtotal(order) + calculateTotalTax(order) - calculateTotalDiscount(order);
    if (payable < 0.0) {
        payable = 0.0;
    }
    return payable;
}

optional<Bill> BillingManager::buildBill(const Order& order, const string& billId) const {
    if (order.getItems().empty()) {
        return nullopt;
    }

    vector<BillItem> billItems;
    billItems.reserve(order.getItems().size());

    for (const auto& item : order.getItems()) {
        auto prod = productManager_.findByBarcode(item.getBarcode());
        if (!prod) {
            return nullopt;
        }

        BillItem bi;
        bi.productId           = prod->getId();
        bi.barcode             = prod->getBarcode();
        bi.productAcronym      = prod->getAcronym();
        bi.unitPriceWithoutTax = prod->getPrice();
        bi.taxPerUnit          = prod->getTax();
        bi.discountPerUnit     = prod->getDiscount();
        bi.quantity            = item.getQuantity();
        billItems.push_back(move(bi));
    }

    // Spec: bill status is the order status (in-progress, on-hold, paid, archived)
    return Bill(billId, order.getCustomerName(), order.getOrderId(), move(billItems),
                toString(order.getOrderStatus()));
}

optional<Bill> BillingManager::generateBill(const Order& order) {
    if (order.getItems().empty()) {
        return nullopt;
    }
    string id = generateBillId();
    auto bill = buildBill(order, id);
    if (!bill) {
        usedBillIds_.erase(id);   // do not burn an ID on a failed bill
    }
    return bill;
}

optional<Bill> BillingManager::previewBill(const Order& order) const {
    return buildBill(order, "");
}

void BillingManager::setPaymentDetails(double amount, PaymentMode mode, const string& merchant) {
    paymentAmount_ = amount;
    paymentMode_ = mode;
    merchantName_ = merchant;
}

bool BillingManager::validatePayment(const Order& order) const {
    if (order.getItems().empty()) {
        return false;
    }
    if (paymentMode_ == PaymentMode::NotSet) {
        return false;
    }
    if (paymentMode_ != PaymentMode::Cash && merchantName_.empty()) {
        return false;
    }

    double payable = calculateTotalPayable(order);
    if (paymentAmount_ + 1e-6 < payable) {
        return false;
    }

    return true;
}

bool BillingManager::updatePaymentStatus(Bill& bill, const string& status) const {
    if (status.empty()) {
        return false;
    }
    bill.setPaymentStatus(status);
    return true;
}

BillingResult BillingManager::finalizeBill(optional<Bill>& outBill) {
    outBill.reset();
    changeReturned_ = 0.0;
    lastPersistenceErrorPath_.clear();

    const Order* current = orderManager_.getCurrentOrder();
    if (!current) {
        return BillingResult::NoActiveOrder;
    }
    if (!current->isLocked()) {
        return BillingResult::OrderNotPlaced;
    }
    if (current->getItems().empty()) {
        return BillingResult::EmptyOrder;
    }

    for (const auto& item : current->getItems()) {
        auto prod = productManager_.findByBarcode(item.getBarcode());
        if (!prod) {
            return BillingResult::ProductNotFound;
        }
        if (!productManager_.checkStock(item.getBarcode(), item.getQuantity())) {
            return BillingResult::InsufficientStock;
        }
    }

    if (!validatePayment(*current)) {
        return BillingResult::InvalidPayment;
    }

    const string billId = generateBillId();
    auto billOpt = buildBill(*current, billId);
    if (!billOpt) {
        usedBillIds_.erase(billId);
        return BillingResult::ProductNotFound;
    }
    Bill bill = move(*billOpt);
    updatePaymentStatus(bill, toString(OrderStatus::Paid));

    // Work on a COPY. The real order stays in OrderManager until everything
    // has been saved, so a failed save never loses the cashier's order.
    Order completed = *current;
    completed.setPayment(bill.getTotalPayable(), paymentMode_, merchantName_);
    completed.setOrderStatus(OrderStatus::Paid);
    const string orderId = completed.getOrderId();

    // What was on disk before (e.g. an on-hold record), so rollback can restore it.
    optional<Order> previous = orderFileHandler_.findOrderById(orderId);

    auto fail = [&](BillingResult result, const string& path) {
        lastPersistenceErrorPath_ = path;
        usedBillIds_.erase(billId);
        return result;
    };
    auto restoreOrderFiles = [&]() {
        if (previous) {
            if (!orderFileHandler_.saveOrder(*previous))
                lastPersistenceErrorPath_ += "\nCleanup also failed: " + orderFileHandler_.orderFilePath();
            const bool detailsOk = previous->getItems().empty()
                ? orderFileHandler_.removeOrderDetails(orderId)
                : orderFileHandler_.saveOrderDetails(*previous);
            if (!detailsOk)
                lastPersistenceErrorPath_ += "\nCleanup also failed: " + orderFileHandler_.orderDetailFilePath();
        } else {
            if (!orderFileHandler_.removeOrderDetails(orderId))
                lastPersistenceErrorPath_ += "\nCleanup also failed: " + orderFileHandler_.orderDetailFilePath();
            if (!orderFileHandler_.removeOrderRecord(orderId))
                lastPersistenceErrorPath_ += "\nCleanup also failed: " + orderFileHandler_.orderFilePath();
        }
    };

    // 1. Order.txt
    if (!orderFileHandler_.saveOrder(completed)) {
        BillingResult r = fail(BillingResult::PersistenceFailed, orderFileHandler_.orderFilePath());
        restoreOrderFiles();
        return r;
    }

    // 2. OrderDetail.txt
    if (!orderFileHandler_.saveOrderDetails(completed)) {
        BillingResult r = fail(BillingResult::PersistenceFailed, orderFileHandler_.orderDetailFilePath());
        restoreOrderFiles();
        return r;
    }

    // 3. Product.txt stock (rolled back if any item fails)
    vector<pair<string, double>> deducted;
    auto rollbackStock = [&]() {
        for (const auto& prev : deducted) {
            if (!productManager_.updateStock(prev.first, prev.second))
                lastPersistenceErrorPath_ += "\nInventory rollback also failed: " + productManager_.getProductFilePath();
        }
    };
    for (const auto& item : completed.getItems()) {
        if (!productManager_.updateStock(item.getBarcode(), -item.getQuantity())) {
            BillingResult r = fail(BillingResult::InventoryUpdateFailed, productManager_.getProductFilePath());
            rollbackStock();
            restoreOrderFiles();
            return r;
        }
        deducted.emplace_back(item.getBarcode(), item.getQuantity());
    }

    // 4. Bill.txt is written LAST, so a failure earlier never leaves a stray bill
    if (!billFileHandler_.saveBill(bill)) {
        BillingResult r = fail(BillingResult::PersistenceFailed, billFileHandler_.billFilePath());
        rollbackStock();
        restoreOrderFiles();
        return r;
    }

    // Everything is on disk: only now release the order from OrderManager.
    orderManager_.completePayment(bill.getTotalPayable(), paymentMode_, merchantName_);

    changeReturned_ = paymentAmount_ - bill.getTotalPayable();
    if (changeReturned_ < 0.0) {
        changeReturned_ = 0.0;
    }

    paymentAmount_ = 0.0;
    paymentMode_ = PaymentMode::NotSet;
    merchantName_.clear();

    outBill = move(bill);
    return BillingResult::Success;
}

BillingResult BillingManager::holdCurrentOrder() {
    lastPersistenceErrorPath_.clear();
    const Order* current = orderManager_.getCurrentOrder();
    if (!current) {
        return BillingResult::NoActiveOrder;
    }
    const string id = current->getOrderId();

    switch (orderManager_.holdOrder()) {
        case OrderResult::Success:       break;
        case OrderResult::NoActiveOrder: return BillingResult::NoActiveOrder;
        case OrderResult::EmptyOrder:    return BillingResult::EmptyOrder;
        default:                         return BillingResult::OrderNotEditable;
    }

    const Order& held = orderManager_.getHeldOrders().at(id);
    if (!orderFileHandler_.saveOrder(held)) {
        lastPersistenceErrorPath_ = orderFileHandler_.orderFilePath();
        orderManager_.resumeOrder(id);          // put it back so nothing is lost
        return BillingResult::PersistenceFailed;
    }
    if (!orderFileHandler_.saveOrderDetails(held)) {
        lastPersistenceErrorPath_ = orderFileHandler_.orderDetailFilePath();
        orderFileHandler_.removeOrderRecord(id);
        orderManager_.resumeOrder(id);
        return BillingResult::PersistenceFailed;
    }
    return BillingResult::Success;
}

OrderResult BillingManager::resumeHeldOrder(const string& orderId) {
    return orderManager_.resumeOrder(orderId);
}

BillingResult BillingManager::cancelCurrentOrder() {
    lastPersistenceErrorPath_.clear();
    const Order* current = orderManager_.getCurrentOrder();
    if (!current) {
        return BillingResult::NoActiveOrder;
    }
    const string id = current->getOrderId();
    orderManager_.cancelOrder();

    // Remove any saved copy (a resumed held order still has an "on-hold" record)
    const bool detailsOk = orderFileHandler_.removeOrderDetails(id);
    const bool recordOk = orderFileHandler_.removeOrderRecord(id);
    if (!detailsOk || !recordOk) {
        lastPersistenceErrorPath_ = !recordOk ? orderFileHandler_.orderFilePath()
                                              : orderFileHandler_.orderDetailFilePath();
        return BillingResult::PersistenceFailed;
    }
    return BillingResult::Success;
}