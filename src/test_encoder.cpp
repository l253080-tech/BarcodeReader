#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <climits>
#include <cstdio>
#include <algorithm>
#include "Barcode_Encoder.h"
#include "Barcode_Decoder.h"
#include "barcode_tables.h"

static int passed = 0, failed = 0;

static void check(bool ok, const std::string& name) {
    if (ok) ++passed;
    else { ++failed; std::cout << "FAIL: " << name << "\n"; }
}

static std::vector<int> toWidths(const std::string& bits, int k) {
    std::vector<int> w;
    size_t i = 0;
    while (i < bits.size()) {
        size_t j = i;
        while (j < bits.size() && bits[j] == bits[i]) ++j;
        w.push_back((int)(j - i) * k);
        i = j;
    }
    return w;
}

int main() {
    std::cout << std::unitbuf;
    std::cout << "Running tests...\n";

    const std::vector<std::string> samples = {
        "123456", "000000", "999999", "000001", "987654", "555555", "100000", "010101"
    };

    // 1. validation
    for (std::string s : {"000000", "123456", "999999", "007007"})
        check(BarcodeEncoder::isValid(s), "valid input accepted: " + s);

    std::vector<std::string> bad = {
        "", "1", "12345", "1234567", "12345a", "a23456", "12 456", " 23456",
        "12345 ", "12-456", "+12345", "12.456", "123\n56", "abcdef", "      ",
        "12345678901234567890",
        std::string("12345\0", 6),
        std::string("12345\xE9")
    };
    for (size_t i = 0; i < bad.size(); ++i)
        check(!BarcodeEncoder::isValid(bad[i]) && BarcodeEncoder::encode(bad[i]).empty(),
            "invalid input #" + std::to_string(i) + " rejected");
    std::cout << "section 1 done\n";

    // 2. checksum known values
    struct CS { const char* s; int v; };
    for (CS c : {CS{ "123456",1 }, CS{ "000000",0 }, CS{ "999999",2 }, CS{ "111111",8 },
        CS{ "000001",9 }, CS{ "100000",7 }, CS{ "654321",5 }})
        check(BarcodeEncoder::checksum(c.s) == c.v, std::string("checksum of ") + c.s);
    std::cout << "section 2 done\n";

    // 3. encode known outputs
    check(BarcodeEncoder::encode("123456") ==
        "1010011001001001101111010100011011000101011110011001101", "encode 123456");
    std::string zeros = "101";
    for (int i = 0; i < 7; ++i) zeros += "0001101";
    zeros += "101";
    check(BarcodeEncoder::encode("000000") == zeros, "encode 000000");
    std::cout << "section 3 done\n";

    // 4. many numbers round-trip (step 997 = about 1000 numbers; use ++n for all 1,000,000)
    {
        char buf[8];
        bool ok = true;
        std::string firstBad;
        for (int n = 0; n <= 999999 && ok; n += 997) {
            std::snprintf(buf, sizeof buf, "%06d", n);
            std::string s = buf;
            std::string bits = BarcodeEncoder::encode(s);
            int cs = BarcodeEncoder::checksum(s);
            bool good = bits.size() == 55
                && bits.find_first_not_of("01") == std::string::npos
                && bits.substr(0, 3) == "101" && bits.substr(52, 3) == "101"
                && cs >= 0 && cs <= 9
                && decodeWidths(toWidths(bits, 1)) == s
                && decodeWidths(toWidths(bits, 3)) == s
                && decodeWidths(toWidths(bits, 7)) == s;
            if (!good) { ok = false; firstBad = s; }
        }
        check(ok, "numbers round-trip (first bad: " + firstBad + ")");
    }
    std::cout << "section 4 done\n";

    // 5. different image scales
    for (const auto& s : samples)
        for (int k : {1, 2, 3, 5, 10, 100, 1000})
            check(decodeWidths(toWidths(BarcodeEncoder::encode(s), k)) == s,
                "scale x" + std::to_string(k) + " for " + s);
    std::cout << "section 5 done\n";

    // 6. noisy widths (+/- 0.2 module)
    {
        std::mt19937 gen(42);
        for (int k : {10, 50}) {
            int amp = k / 5;
            std::uniform_int_distribution<int> jit(-amp, amp);
            for (const auto& s : samples) {
                bool ok = true;
                for (int t = 0; t < 200; ++t) {
                    auto w = toWidths(BarcodeEncoder::encode(s), k);
                    for (int& x : w) x += jit(gen);
                    if (decodeWidths(w) != s) ok = false;
                }
                check(ok, "noise at k=" + std::to_string(k) + " for " + s);
            }
        }
    }
    std::cout << "section 6 done\n";

    // 7. one flipped module must never give a number
    for (const auto& s : samples) {
        std::string bits = BarcodeEncoder::encode(s);
        for (int pos = 0; pos < 55; ++pos) {
            std::string f = bits;
            f[pos] = f[pos] == '0' ? '1' : '0';
            check(decodeWidths(toWidths(f, 3)).empty(),
                "flip module " + std::to_string(pos) + " of " + s);
        }
    }
    std::cout << "section 7 done\n";

    // 8. broken structure
    {
        std::string good = BarcodeEncoder::encode("123456");

        std::string wrongCs = "101";
        for (char c : std::string("123456")) wrongCs += DIGIT_PATTERN[c - '0'];
        wrongCs += DIGIT_PATTERN[2];
        wrongCs += "101";
        check(decodeWidths(toWidths(wrongCs, 3)).empty(), "wrong checksum rejected");

        std::string b = good; b.replace(0, 3, "111");
        check(decodeWidths(toWidths(b, 3)).empty(), "start 111 rejected");
        b = good; b.replace(0, 3, "100");
        check(decodeWidths(toWidths(b, 3)).empty(), "start 100 rejected");
        b = good; b.replace(52, 3, "111");
        check(decodeWidths(toWidths(b, 3)).empty(), "stop 111 rejected");
        b = good; b.replace(52, 3, "100");
        check(decodeWidths(toWidths(b, 3)).empty(), "stop 100 rejected");
        b = good; b.replace(3, 7, "0000000");
        check(decodeWidths(toWidths(b, 3)).empty(), "unknown pattern 0000000 rejected");
        b = good; b.replace(3, 7, "1111111");
        check(decodeWidths(toWidths(b, 3)).empty(), "unknown pattern 1111111 rejected");
    }
    std::cout << "section 8 done\n";

    // 9. bad width lists
    {
        auto valid = toWidths(BarcodeEncoder::encode("123456"), 3);

        check(decodeWidths({}).empty(), "empty list");
        check(decodeWidths({ 5 }).empty(), "1 width");
        check(decodeWidths({ 5, 5 }).empty(), "2 widths");
        check(decodeWidths({ 0, 0, 0 }).empty(), "all zeros (3)");
        check(decodeWidths(std::vector<int>(55, 0)).empty(), "all zeros (55)");
        check(decodeWidths({ INT_MAX, INT_MAX, INT_MAX }).empty(), "INT_MAX widths");
        check(decodeWidths({ -3, -3, -3 }).empty(), "negative widths");
        check(decodeWidths(std::vector<int>(1000, 1)).empty(), "1000 stripes");
        check(decodeWidths(std::vector<int>(56, 3)).empty(), "56 stripes");
        check(decodeWidths(std::vector<int>(55, 5)).empty(), "55 equal stripes");
        check(decodeWidths(std::vector<int>(55, 1)).empty(), "55 one-pixel stripes");
        check(decodeWidths({ 1, 1, 1 }).empty(), "3 stripes of 1");

        auto v = valid; v[0] = -3;
        check(decodeWidths(v).empty(), "valid list with a negative width");
        v = valid; v[10] = 0;
        check(decodeWidths(v).empty(), "valid list with a zero width");
        v = valid; v.pop_back();
        check(decodeWidths(v).empty(), "last stripe missing");
        v = valid; v.erase(v.begin());
        check(decodeWidths(v).empty(), "first stripe missing");

        v = valid; std::reverse(v.begin(), v.end());
        check(decodeWidths(v).empty(), "reversed list returns empty");

        v = valid; v.insert(v.begin(), 30);
        std::string r = decodeWidths(v);
        check(r.empty() || r == "123456", "left margin never wrong");
        v = valid; v.push_back(30);
        r = decodeWidths(v);
        check(r.empty() || r == "123456", "right margin never wrong");
    }
    std::cout << "section 9 done\n";

    std::cout << "\nPassed: " << passed << "   Failed: " << failed << "\n";
    if (failed == 0) std::cout << "ALL TESTS PASSED\n";
    return failed == 0 ? 0 : 1;
}