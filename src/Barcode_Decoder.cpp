#include "Barcode_Decoder.h"
#include "Barcode_Tabels.h"
#include <cmath>
#include <algorithm>

// Reads one symbol (6 elements, or 7 for the stop) and returns its value 0..106, or -1.
static int readSymbol(const std::vector<int>& w, size_t start, size_t count, int modules) {
    double total = 0;
    for (size_t i = 0; i < count; ++i) total += w[start + i];
    double module = total / modules;               // pixels per module for this symbol

    std::string key;
    int sum = 0;
    for (size_t i = 0; i < count; ++i) {
        int n = (int)std::lround(w[start + i] / module);
        if (n < 1 || n > 4) return -1;
        key += char('0' + n);
        sum += n;
    }
    if (sum != modules) return -1;

    for (int v = 0; v < (int)CODE128_WIDTHS.size(); ++v)
        if (CODE128_WIDTHS[v] == key) return v;
    return -1;
}

static std::string decodeForward(const std::vector<int>& widths) {
    if (widths.size() != (size_t)TOTAL_STRIPES) return "";
    for (int w : widths)
        if (w <= 0) return "";

    int v[5];
    for (int s = 0; s < 5; ++s) {
        v[s] = readSymbol(widths, s * 6, 6, SYMBOL_MODULES);
        if (v[s] < 0) return "";
    }
    if (readSymbol(widths, 30, 7, STOP_MODULES) != STOP) return "";

    if (v[0] != START_C) return "";
    for (int i = 1; i <= 3; ++i)
        if (v[i] > 99) return "";                   // data pairs must be 00..99
    if (v[4] > 102) return "";

    int sum = START_C + 1 * v[1] + 2 * v[2] + 3 * v[3];
    if (sum % 103 != v[4]) return "";

    std::string digits;
    for (int i = 1; i <= 3; ++i) {
        digits += char('0' + v[i] / 10);
        digits += char('0' + v[i] % 10);
    }
    return digits;
}

std::string decodeWidths(const std::vector<int>& widths) {
    std::string r = decodeForward(widths);
    if (!r.empty()) return r;
    std::vector<int> rev(widths.rbegin(), widths.rend());   // barcode held upside down
    return decodeForward(rev);
}