#pragma once
#include <string>
#include <vector>

// widths: alternating bar, space, bar... in pixels, starting with the first black bar
// and ending with the last black bar (37 values). Also works if the barcode is read right to left.
// Returns the 6 digits, or "" if decoding fails.
std::string decodeWidths(const std::vector<int>& widths);