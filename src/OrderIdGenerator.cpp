#include "OrderIdGenerator.h"
#include <cctype>
#include <cstdio>
#include <stdexcept>

void OrderIdGenerator::registerExistingId(const std::string& id) {
    if (id.size() != 8) return;
    for (char c : id) if (!std::isdigit((unsigned char)c)) return;
    used_.insert(id);
}

std::string OrderIdGenerator::generate(int dayOfMonth) {
    if (dayOfMonth < 1 || dayOfMonth > 31)
        throw std::invalid_argument("day of month must be 1..31");

    for (int seq = 1; seq <= 999999; ++seq) {
        char buf[16];
        std::snprintf(buf, sizeof buf, "%06d%02d", seq, dayOfMonth);
        if (used_.insert(buf).second) return buf;
    }
    throw std::runtime_error("no free order IDs left for this day");
}
