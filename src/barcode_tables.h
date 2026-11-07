#pragma once
#include <array>
#include <string>

// Stripe pattern for each digit 0-9 (1 = black, 0 = white), 7 modules each
const std::array<std::string, 10> DIGIT_PATTERN = {
    "0001101", "0011001", "0010011", "0111101", "0100011",
    "0110001", "0101111", "0111011", "0110111", "0001011"
};

const std::string START_GUARD = "101";
const std::string STOP_GUARD = "101";

// start (3) + 7 digit patterns (6 digits + checksum) + stop (3)
const int TOTAL_MODULES = 3 + 7 * 7 + 3; // 55 