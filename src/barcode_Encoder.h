#pragma once
#include <string>

class BarcodeEncoder {
public:
    static bool isValid(const std::string& s);
    static int checksum(const std::string& s);
    static std::string encode(const std::string& s);
};