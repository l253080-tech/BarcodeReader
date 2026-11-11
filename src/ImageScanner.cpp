// #include "ImageScanner.h"

// #include <QPainter>
// #include <algorithm>
// #include <cmath>
// #include <cstdint>
// #include <map>

// using namespace std;

// namespace
// {
//     // ---- Settings ------------------------------------------------------------
//     const int kMinWidth = 800;          // smaller images are enlarged to this width
//     const int kTotalModules = 68;       // a barcode is exactly 68 modules wide
//     const size_t kExpectedRuns = 37;    // 6+6+6+6+6+7 bars and spaces
//     // Scan lines (fraction of image height): middle first, then outward.
//     const double kRows[] = {0.50, 0.40, 0.60, 0.30, 0.70, 0.20, 0.80, 0.15, 0.85};
//     // --------------------------------------------------------------------------

//     struct Run
//     {
//         bool black;
//         int width;
//     };

//     // Transparent PNGs become white (not black); small images are enlarged;
//     // result is 8-bit grayscale.
//     QImage toGrayscale(const QImage &input)
//     {
//         QImage img = input;

//         if (img.hasAlphaChannel())
//         {
//             QImage flat(img.size(), QImage::Format_RGB32);
//             flat.fill(Qt::white);
//             QPainter p(&flat);
//             p.drawImage(0, 0, img);
//             p.end();
//             img = flat;
//         }

//         if (img.width() < kMinWidth)
//             img = img.scaledToWidth(kMinWidth, Qt::SmoothTransformation);

//         return img.convertToFormat(QImage::Format_Grayscale8);
//     }

//     // Otsu's method: best global black/white threshold from the histogram.
//     int otsuThreshold(const QImage &gray)
//     {
//         long long hist[256] = {0};
//         for (int y = 0; y < gray.height(); ++y)
//         {
//             const uchar *line = gray.constScanLine(y);
//             for (int x = 0; x < gray.width(); ++x)
//                 ++hist[line[x]];
//         }

//         const long long total = static_cast<long long>(gray.width()) * gray.height();
//         double sumAll = 0;
//         for (int i = 0; i < 256; ++i)
//             sumAll += static_cast<double>(i) * hist[i];

//         double sumB = 0, maxVar = 0;
//         long long wB = 0;
//         int threshold = 128;

//         for (int t = 0; t < 256; ++t)
//         {
//             wB += hist[t];
//             if (wB == 0)
//                 continue;
//             const long long wF = total - wB;
//             if (wF == 0)
//                 break;

//             sumB += static_cast<double>(t) * hist[t];
//             const double mB = sumB / wB;
//             const double mF = (sumAll - sumB) / wF;
//             const double var = static_cast<double>(wB) * wF * (mB - mF) * (mB - mF);
//             if (var > maxVar)
//             {
//                 maxVar = var;
//                 threshold = t;
//             }
//         }
//         return threshold;
//     }

//     vector<int> rowPixels(const QImage &gray, int y)
//     {
//         vector<int> row(gray.width());
//         const uchar *line = gray.constScanLine(y);
//         for (int x = 0; x < gray.width(); ++x)
//             row[x] = line[x];
//         return row;
//     }

//     // Light smoothing along the line: removes pixel noise, helps blurry images.
//     vector<int> smoothRow(const vector<int> &r)
//     {
//         const int n = static_cast<int>(r.size());
//         vector<int> out(n);
//         for (int i = 0; i < n; ++i)
//         {
//             const int a = r[max(0, i - 1)];
//             const int b = r[i];
//             const int c = r[min(n - 1, i + 1)];
//             out[i] = (a + b + c) / 3;
//         }
//         return out;
//     }

//     // One fixed threshold for the whole line (1 = black).
//     vector<uint8_t> binarizeGlobal(const vector<int> &row, int threshold)
//     {
//         vector<uint8_t> out(row.size());
//         for (size_t i = 0; i < row.size(); ++i)
//             out[i] = row[i] < threshold ? 1 : 0;
//         return out;
//     }

//     // Adaptive threshold: compare each pixel with the mean of its neighbourhood.
//     // Works when one side of the image is darker than the other.
//     vector<uint8_t> binarizeAdaptive(const vector<int> &row)
//     {
//         const int n = static_cast<int>(row.size());
//         const int half = max(8, n / 16);

//         vector<long long> pre(n + 1, 0);
//         for (int i = 0; i < n; ++i)
//             pre[i + 1] = pre[i] + row[i];

//         vector<uint8_t> out(n);
//         for (int i = 0; i < n; ++i)
//         {
//             const int lo = max(0, i - half);
//             const int hi = min(n, i + half + 1);
//             const double mean = static_cast<double>(pre[hi] - pre[lo]) / (hi - lo);
//             out[i] = row[i] < mean * 0.85 ? 1 : 0;
//         }
//         return out;
//     }

//     // Count consecutive pixels of the same colour -> list of runs.
//     vector<Run> toRuns(const vector<uint8_t> &bin)
//     {
//         vector<Run> runs;
//         if (bin.empty())
//             return runs;

//         bool cur = bin[0] != 0;
//         int len = 1;
//         for (size_t i = 1; i < bin.size(); ++i)
//         {
//             const bool b = bin[i] != 0;
//             if (b == cur)
//                 ++len;
//             else
//             {
//                 runs.push_back({cur, len});
//                 cur = b;
//                 len = 1;
//             }
//         }
//         runs.push_back({cur, len});
//         return runs;
//     }

//     // Remove tiny specks: white | black speck | white -> one white run.
//     vector<Run> removeSpecks(vector<Run> runs, int minRun)
//     {
//         while (runs.size() > 1 && runs.front().width < minRun)
//             runs.erase(runs.begin());
//         while (runs.size() > 1 && runs.back().width < minRun)
//             runs.pop_back();

//         bool changed = true;
//         while (changed)
//         {
//             changed = false;
//             for (size_t i = 1; i + 1 < runs.size(); ++i)
//             {
//                 if (runs[i].width < minRun)
//                 {
//                     runs[i - 1].width += runs[i].width + runs[i + 1].width;
//                     runs.erase(runs.begin() + i, runs.begin() + i + 2);
//                     changed = true;
//                     break;
//                 }
//             }
//         }
//         return runs;
//     }

//     // Cut off the white quiet zones so we start and end with a black bar.
//     vector<Run> cleanRuns(const vector<uint8_t> &bin, int minRun)
//     {
//         vector<Run> runs = removeSpecks(toRuns(bin), minRun);
//         if (!runs.empty() && !runs.front().black)
//             runs.erase(runs.begin());
//         if (!runs.empty() && !runs.back().black)
//             runs.pop_back();
//         return runs;
//     }

//     // Pixel runs -> module widths. Returns empty if the result is not valid
//     // (wrong number of runs, or widths do not add up to 68 modules).
//     vector<int> toModules(const vector<Run> &runs)
//     {
//         if (runs.size() != kExpectedRuns)
//             return {};

//         long long span = 0; // first black pixel to last black pixel
//         for (const Run &r : runs)
//             span += r.width;
//         if (span <= 0)
//             return {};

//         const double module = static_cast<double>(span) / kTotalModules;

//         vector<int> out;
//         out.reserve(runs.size());
//         int sum = 0;
//         for (const Run &r : runs)
//         {
//             const int w = max(1, static_cast<int>(lround(r.width / module)));
//             out.push_back(w);
//             sum += w;
//         }

//         if (sum != kTotalModules)
//             return {};
//         return out;
//     }

// } // namespace

// vector<int> ImageScanner::getWidths(const QImage &image)
// {
//     if (image.isNull())
//         return {};

//     const QImage gray = toGrayscale(image);
//     if (gray.isNull() || gray.height() < 1)
//         return {};

//     const int globalThreshold = otsuThreshold(gray);
//     const int minRun = max(2, gray.width() / 400);

//     // Every valid reading gets one vote; the most common reading wins.
//     map<vector<int>, int> votes;

//     for (double f : kRows)
//     {
//         const int y = static_cast<int>(f * (gray.height() - 1));
//         const vector<int> row = smoothRow(rowPixels(gray, y));

//         for (int mode = 0; mode < 2; ++mode)
//         {
//             const vector<uint8_t> bin = (mode == 0)
//             ? binarizeGlobal(row, globalThreshold)
//             : binarizeAdaptive(row);

//             const vector<int> widths = toModules(cleanRuns(bin, minRun));
//             if (!widths.empty())
//                 ++votes[widths];
//         }
//     }

//     vector<int> best;
//     int bestVotes = 0;
//     for (const auto &kv : votes)
//     {
//         if (kv.second > bestVotes)
//         {
//             best = kv.first;
//             bestVotes = kv.second;
//         }
//     }
//     return best; // empty if no line gave a valid barcode
// }

#include "ImageScanner.h"
#include "Barcode_Decoder.h"
#include <QPainter>
#include <QTransform>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <string>

using namespace std;

namespace
{
// ---- Settings ------------------------------------------------------------
const int kMinWidth = 800;          // smaller images are enlarged to this width
const size_t kExpectedRuns = 37;    // 6+6+6+6+6+7 bars and spaces
const int kScanLines = 19;          // scan lines at 5%, 10% ... 95% of the height
// Small rotations are tried too: a hand-held phone is rarely perfectly straight.
const double kAngles[] = {0, -3, 3, -6, 6, -10, 10, -15, 15};
// --------------------------------------------------------------------------

struct Run
{
    bool black;
    int width;
};

// One valid barcode found on a scan line.
struct Reading
{
    string digits;          // the 6 decoded digits
    vector<int> widths;     // the 37 bar/space widths in pixels
};

// Transparent PNGs become white (not black); small images are enlarged;
// result is 8-bit grayscale.
QImage toGrayscale(const QImage &input)
{
    QImage img = input;

    if (img.hasAlphaChannel())
    {
        QImage flat(img.size(), QImage::Format_RGB32);
        flat.fill(Qt::white);
        QPainter p(&flat);
        p.drawImage(0, 0, img);
        p.end();
        img = flat;
    }

    if (img.width() < kMinWidth)
        img = img.scaledToWidth(kMinWidth, Qt::SmoothTransformation);

    return img.convertToFormat(QImage::Format_Grayscale8);
}

// Otsu's method: best global black/white threshold from the histogram.
int otsuThreshold(const QImage &gray)
{
    long long hist[256] = {0};
    for (int y = 0; y < gray.height(); ++y)
    {
        const uchar *line = gray.constScanLine(y);
        for (int x = 0; x < gray.width(); ++x)
            ++hist[line[x]];
    }

    const long long total = static_cast<long long>(gray.width()) * gray.height();
    double sumAll = 0;
    for (int i = 0; i < 256; ++i)
        sumAll += static_cast<double>(i) * hist[i];

    double sumB = 0, maxVar = 0;
    long long wB = 0;
    int threshold = 128;

    for (int t = 0; t < 256; ++t)
    {
        wB += hist[t];
        if (wB == 0)
            continue;
        const long long wF = total - wB;
        if (wF == 0)
            break;

        sumB += static_cast<double>(t) * hist[t];
        const double mB = sumB / wB;
        const double mF = (sumAll - sumB) / wF;
        const double var = static_cast<double>(wB) * wF * (mB - mF) * (mB - mF);
        if (var > maxVar)
        {
            maxVar = var;
            threshold = t;
        }
    }
    return threshold;
}

vector<int> rowPixels(const QImage &gray, int y)
{
    vector<int> row(gray.width());
    const uchar *line = gray.constScanLine(y);
    for (int x = 0; x < gray.width(); ++x)
        row[x] = line[x];
    return row;
}

// Light smoothing along the line: removes pixel noise, helps blurry images.
vector<int> smoothRow(const vector<int> &r)
{
    const int n = static_cast<int>(r.size());
    vector<int> out(n);
    for (int i = 0; i < n; ++i)
    {
        const int a = r[max(0, i - 1)];
        const int b = r[i];
        const int c = r[min(n - 1, i + 1)];
        out[i] = (a + b + c) / 3;
    }
    return out;
}

// One fixed threshold for the whole line (1 = black).
vector<uint8_t> binarizeGlobal(const vector<int> &row, int threshold)
{
    vector<uint8_t> out(row.size());
    for (size_t i = 0; i < row.size(); ++i)
        out[i] = row[i] < threshold ? 1 : 0;
    return out;
}

// Adaptive threshold: compare each pixel with the mean of its neighbourhood.
// Works when one side of the image is darker than the other.
vector<uint8_t> binarizeAdaptive(const vector<int> &row)
{
    const int n = static_cast<int>(row.size());
    const int half = max(8, n / 16);

    vector<long long> pre(n + 1, 0);
    for (int i = 0; i < n; ++i)
        pre[i + 1] = pre[i] + row[i];

    vector<uint8_t> out(n);
    for (int i = 0; i < n; ++i)
    {
        const int lo = max(0, i - half);
        const int hi = min(n, i + half + 1);
        const double mean = static_cast<double>(pre[hi] - pre[lo]) / (hi - lo);
        out[i] = row[i] < mean * 0.85 ? 1 : 0;
    }
    return out;
}

// Count consecutive pixels of the same colour -> list of runs.
vector<Run> toRuns(const vector<uint8_t> &bin)
{
    vector<Run> runs;
    if (bin.empty())
        return runs;

    bool cur = bin[0] != 0;
    int len = 1;
    for (size_t i = 1; i < bin.size(); ++i)
    {
        const bool b = bin[i] != 0;
        if (b == cur)
            ++len;
        else
        {
            runs.push_back({cur, len});
            cur = b;
            len = 1;
        }
    }
    runs.push_back({cur, len});
    return runs;
}

// Remove tiny specks: white | black speck | white -> one white run.
vector<Run> removeSpecks(vector<Run> runs, int minRun)
{
    while (runs.size() > 1 && runs.front().width < minRun)
        runs.erase(runs.begin());
    while (runs.size() > 1 && runs.back().width < minRun)
        runs.pop_back();

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (size_t i = 1; i + 1 < runs.size(); ++i)
        {
            if (runs[i].width < minRun)
            {
                runs[i - 1].width += runs[i].width + runs[i + 1].width;
                runs.erase(runs.begin() + i, runs.begin() + i + 2);
                changed = true;
                break;
            }
        }
    }
    return runs;
}

// Remove specks only. The barcode can be anywhere on the line (a camera
// picture also contains table, phone edge, text ...), so we do NOT cut the
// line down to exactly 37 runs here. findBarcodes() looks for the barcode.
vector<Run> cleanRuns(const vector<uint8_t> &bin, int minRun)
{
    return removeSpecks(toRuns(bin), minRun);
}

// Slide a window of 37 runs (black ... black) along the line and keep every
// window that really decodes (start code, patterns and checksum are checked
// by decodeWidths). The pixel widths are passed on directly: the decoder
// measures the module size per symbol, so it also copes with a barcode that
// is slightly smaller on one side (phone held at an angle).
vector<Reading> findBarcodes(const vector<Run> &runs)
{
    vector<Reading> found;
    for (size_t i = 0; i + kExpectedRuns <= runs.size(); ++i)
    {
        if (!runs[i].black)
            continue;

        vector<int> w;
        w.reserve(kExpectedRuns);
        for (size_t k = 0; k < kExpectedRuns; ++k)
            w.push_back(runs[i + k].width);

        const string digits = decodeWidths(w);
        if (!digits.empty())
            found.push_back({digits, w});
    }
    return found;
}

// Scan one image on many lines. Returns the widths of the most common valid
// reading, or an empty vector if no line contained a valid barcode.
vector<int> scanOne(const QImage &image)
{
    const QImage gray = toGrayscale(image);
    if (gray.isNull() || gray.height() < 1)
        return {};

    const int globalThreshold = otsuThreshold(gray);
    const int minRun = max(2, gray.width() / 400);

    // Every valid reading gets one vote; the most common digits win.
    map<string, int> votes;
    map<string, vector<int>> widthsOf;

    for (int step = 1; step <= kScanLines; ++step)
    {
        const int y = static_cast<int>(
            (static_cast<double>(step) / (kScanLines + 1)) * (gray.height() - 1));
        const vector<int> row = smoothRow(rowPixels(gray, y));

        for (int mode = 0; mode < 2; ++mode)
        {
            const vector<uint8_t> bin = (mode == 0)
            ? binarizeGlobal(row, globalThreshold)
            : binarizeAdaptive(row);

            for (const Reading &r : findBarcodes(cleanRuns(bin, minRun)))
            {
                ++votes[r.digits];
                if (!widthsOf.count(r.digits))
                    widthsOf[r.digits] = r.widths;
            }
        }
    }

    string bestDigits;
    int bestVotes = 0;
    for (const auto &kv : votes)
    {
        if (kv.second > bestVotes)
        {
            bestDigits = kv.first;
            bestVotes = kv.second;
        }
    }
    if (bestVotes == 0)
        return {};
    return widthsOf[bestDigits];
}

} // namespace

vector<int> ImageScanner::getWidths(const QImage &image)
{
    if (image.isNull())
        return {};

    // ARGB so the empty corners created by a rotation turn white in toGrayscale().
    const QImage argb = image.convertToFormat(QImage::Format_ARGB32);

    for (double angle : kAngles)
    {
        const QImage img = (angle == 0.0)
        ? image
        : argb.transformed(QTransform().rotate(angle), Qt::SmoothTransformation);

        const vector<int> widths = scanOne(img);
        if (!widths.empty())
            return widths;
    }
    return {}; // empty if no angle gave a valid barcode
}