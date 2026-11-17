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
    for (const auto& existingOrder : orderFileHandler_.loadOrders()) {
        orderManager_.registerExistingOrderId(existingOrder.getOrderId());
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

optional<Bill> BillingManager::generateBill(const Order& order) {
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

    string status = "unpaid";
    if (order.getOrderStatus() == OrderStatus::Paid) {
        status = "paid";
    }
    Bill bill(generateBillId(), order.getCustomerName(), order.getOrderId(), move(billItems), status);
    return bill;
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

    auto billOpt = generateBill(*current);
    if (!billOpt) {
        return BillingResult::ProductNotFound;
    }

    Bill bill = move(*billOpt);
    updatePaymentStatus(bill, "paid");

    auto completedOrderOpt = orderManager_.completePayment(paymentAmount_, paymentMode_, merchantName_);
    if (!completedOrderOpt) {
        return BillingResult::OrderNotPlaced;
    }
    Order completedOrder = move(*completedOrderOpt);

    if (!orderFileHandler_.saveOrder(completedOrder)) {
        return BillingResult::PersistenceFailed;
    }

    if (!orderFileHandler_.saveOrderDetails(completedOrder)) {
        orderFileHandler_.removeOrderRecord(completedOrder.getOrderId());
        return BillingResult::PersistenceFailed;
    }

    if (!billFileHandler_.saveBill(bill)) {
        orderFileHandler_.removeOrderDetails(completedOrder.getOrderId());
        orderFileHandler_.removeOrderRecord(completedOrder.getOrderId());
        return BillingResult::PersistenceFailed;
    }

    vector<pair<string, double>> deducted;
    for (const auto& item : completedOrder.getItems()) {
        if (!productManager_.updateStock(item.getBarcode(), -item.getQuantity())) {
            for (const auto& prev : deducted) {
                productManager_.updateStock(prev.first, prev.second);
            }
            orderFileHandler_.removeOrderDetails(completedOrder.getOrderId());
            orderFileHandler_.removeOrderRecord(completedOrder.getOrderId());
            return BillingResult::InventoryUpdateFailed;
        }
        deducted.emplace_back(item.getBarcode(), item.getQuantity());
    }

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