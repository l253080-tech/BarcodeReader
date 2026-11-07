#include "Barcode_Decoder.h"
#include "Barcode_Encoder.h"
#include "barcode_tables.h"
#include <cmath>

std::string decodeWidths(const std::vector<int>& widths) {
    if (widths.size() < 3 || widths.size() > (size_t)TOTAL_MODULES) return "";
    double total = 0;
    for (int w : widths) {
        if (w <= 0) return "";
        total += w;
    }
    double module = total / TOTAL_MODULES;
    std::string bits;
    for (size_t i = 0; i < widths.size(); ++i) {
        int n = (int)std::lround(widths[i] / module);
        if (n < 1) n = 1;
        bits += std::string(n, i % 2 == 0 ? '1' : '0');
    }
    if ((int)bits.size() != TOTAL_MODULES) return "";

    if (bits.substr(0, 3) != START_GUARD) return "";
    if (bits.substr(52, 3) != STOP_GUARD) return "";

    std::string digits;
    for (int d = 0; d < 7; ++d) {
        std::string chunk = bits.substr(3 + d * 7, 7);
        int match = -1;
        for (int k = 0; k < 10; ++k)
            if (DIGIT_PATTERN[k] == chunk) match = k;
        if (match < 0) return "";
        digits += char('0' + match);
    }

    std::string data = digits.substr(0, 6);
    if (BarcodeEncoder::checksum(data) != digits[6] - '0') return "";
    return data;
}