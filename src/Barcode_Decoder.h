#pragma once
#include <string>
#include <vector>

// widths: alternating bar, space, bar... in pixels.
// Returns the 6 digits, or "" if decoding fails.
std::string decodeWidths(const std::vector<int>& widths);