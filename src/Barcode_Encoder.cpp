#include "Barcode_Encoder.h"
#include "Barcode_Tabels.h"
#include <cctype>

bool BarcodeEncoder::isValid(const std::string& s) {
    if (s.size() != 6) return false;
    for (char c : s)
        if (!std::isdigit((unsigned char)c)) return false;
    return true;
}

// Code 128 checksum: (105 + 1*v1 + 2*v2 + 3*v3) mod 103, where v1..v3 are the digit pairs
int BarcodeEncoder::checksum(const std::string& s) {
    if (s.size() != 6) return -1;
    int sum = START_C;
    for (int i = 0; i < 3; ++i) {
        int value = (s[2 * i] - '0') * 10 + (s[2 * i + 1] - '0');
        sum += (i + 1) * value;
    }
    return sum % 103;
}

static void appendSymbol(std::string& out, int value) {
    const std::string& w = CODE128_WIDTHS[value];
    for (size_t i = 0; i < w.size(); ++i)
        out += std::string(w[i] - '0', i % 2 == 0 ? '1' : '0');
}

// returns 68 chars of '0'/'1' (1 = black bar), or "" if the input is not 6 digits
std::string BarcodeEncoder::encode(const std::string& s) {
    if (!isValid(s)) return "";
    std::string out;
    appendSymbol(out, START_C);
    for (int i = 0; i < 3; ++i)
        appendSymbol(out, (s[2 * i] - '0') * 10 + (s[2 * i + 1] - '0'));
    appendSymbol(out, checksum(s));
    appendSymbol(out, STOP);
    return out;
}   

































































