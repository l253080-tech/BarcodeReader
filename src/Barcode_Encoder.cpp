#include "Barcode_Encoder.h"
#include "barcode_tables.h"
#include <cctype>

bool BarcodeEncoder::isValid(const std::string& s) {
    if (s.size() != 6) return false;
    for (char c : s)
        if (!std::isdigit((unsigned char)c)) return false;
    return true;
}

int BarcodeEncoder::checksum(const std::string& s) {
    int sum = 0;
    for (size_t i = 0; i < s.size(); ++i)
        sum += (s[i] - '0') * (i % 2 == 0 ? 3 : 1);
    return (10 - sum % 10) % 10;
}

std::string BarcodeEncoder::encode(const std::string& s) {
    if (!isValid(s)) return "";
    std::string out = START_GUARD;
    for (char c : s)
        out += DIGIT_PATTERN[c - '0'];
    out += DIGIT_PATTERN[checksum(s)];
    out += STOP_GUARD;
    return out;
}