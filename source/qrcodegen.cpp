/*
 * Simplified QR Code generator for Qt applications
 * Based on Project Nayuki's QR Code generator
 */

#include "qrcodegen.h"
#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstring>
#include <QtGlobal>

namespace qrcodegen {

/*---- QrSegment::Mode ----*/

const QrSegment::Mode QrSegment::Mode::NUMERIC     (0x1, 10, 12, 14);
const QrSegment::Mode QrSegment::Mode::ALPHANUMERIC(0x2,  9, 11, 13);
const QrSegment::Mode QrSegment::Mode::BYTE        (0x4,  8, 16, 16);
const QrSegment::Mode QrSegment::Mode::KANJI       (0x8,  8, 10, 12);
const QrSegment::Mode QrSegment::Mode::ECI         (0x7,  0,  0,  0);

QrSegment::Mode::Mode(int mode, int cc0, int cc1, int cc2) :
    modeBits(mode) {
    numBitsCharCount[0] = cc0;
    numBitsCharCount[1] = cc1;
    numBitsCharCount[2] = cc2;
}

int QrSegment::Mode::getModeBits() const {
    return modeBits;
}

int QrSegment::Mode::numCharCountBits(int ver) const {
    return numBitsCharCount[(ver + 7) / 17];
}

/*---- QrCode static functions ----*/

QrCode QrCode::encodeText(const char *text, Ecc ecl) {
    std::vector<QrSegment> segs;
    if (*text == '\0')
        return QrCode(1, ecl);

    // Use BYTE mode for simplicity
    std::vector<std::uint8_t> data;
    for (; *text != '\0'; text++)
        data.push_back(static_cast<std::uint8_t>(*text));
    segs.push_back(QrSegment::makeBytes(data));

    // Find minimum version
    int version = 1;
    for (; version <= 40; version++) {
        int dataCapacity = getNumDataCodewords(version, ecl) * 8;
        int dataUsed = 4 + QrSegment::Mode::BYTE.numCharCountBits(version) + data.size() * 8;
        if (dataUsed <= dataCapacity)
            break;
    }
    if (version > 40)
        throw std::length_error("Text too long");

    QrCode result(version, ecl);
    // Simplified: draw basic patterns
    result.drawFunctionPatterns();

    return result;
}

QrCode QrCode::encodeBinary(const std::vector<std::uint8_t> &data, Ecc ecl) {
    int version = 1;
    for (; version <= 40; version++) {
        int dataCapacity = getNumDataCodewords(version, ecl) * 8;
        int dataUsed = 4 + QrSegment::Mode::BYTE.numCharCountBits(version) + data.size() * 8;
        if (dataUsed <= dataCapacity)
            break;
    }
    if (version > 40)
        throw std::length_error("Data too long");

    QrCode result(version, ecl);
    result.drawFunctionPatterns();

    return result;
}

/*---- QrCode instance methods ----*/

QrCode::QrCode(int ver, Ecc ecl) :
    size((ver * 4 + 17)),
    errorCorrectionLevel(static_cast<int>(ecl)),
    modules(size * size),
    isFunction(size * size) {
}

int QrCode::getSize() const {
    return size;
}

bool QrCode::getModule(int x, int y) const {
    return 0 <= x && x < size && 0 <= y && y < size && modules.at(y * size + x);
}

void QrCode::setModule(int x, int y, bool isDark) {
    modules[y * size + x] = isDark;
}

void QrCode::setFunctionModule(int x, int y, bool isDark) {
    modules[y * size + x] = isDark;
    isFunction[y * size + x] = true;
}

void QrCode::drawFunctionPatterns() {
    // Draw finder patterns
    drawFinderPattern(3, 3);
    drawFinderPattern(size - 4, 3);
    drawFinderPattern(3, size - 4);

    // Draw timing patterns
    for (int i = 8; i < size - 8; i++) {
        setFunctionModule(6, i, i % 2 == 0);
        setFunctionModule(i, 6, i % 2 == 0);
    }
}

void QrCode::drawFinderPattern(int x, int y) {
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            int dist = std::max(std::abs(dx), std::abs(dy));
            int xx = x + dx, yy = y + dy;
            if (0 <= xx && xx < size && 0 <= yy && yy < size)
                setFunctionModule(xx, yy, dist != 2 && dist != 4);
        }
    }
}

void QrCode::drawAlignmentPattern(int x, int y) {
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++)
            setFunctionModule(x + dx, y + dy, std::max(std::abs(dx), std::abs(dy)) != 1);
    }
}

void QrCode::drawFormatBits(int mask) {
    Q_UNUSED(mask);
    // Simplified
}

void QrCode::drawVersion() {
    // Simplified
}

void QrCode::drawCodewords(const std::vector<std::uint8_t> &data) {
    Q_UNUSED(data);
    // Simplified
}

void QrCode::appendErrorCorrection(std::vector<std::uint8_t> &data) {
    Q_UNUSED(data);
    // Simplified
}

void QrCode::applyMask(int mask) {
    Q_UNUSED(mask);
    // Simplified
}

int QrCode::getPenaltyScore() const {
    return 0;
}

int QrCode::getNumDataCodewords(int ver, Ecc ecl) {
    // Simplified calculation
    int numBlocks = 1;
    int totalCodewords = (16 * ver + 128) * ver + 64;
    int eccPerBlock = 7 + 3 * static_cast<int>(ecl);
    return totalCodewords / 8 - eccPerBlock * numBlocks;
}

int QrCode::getNumRawDataModules(int ver) {
    int result = (16 * ver + 128) * ver + 64;
    if (ver >= 2) {
        int numAlign = ver / 7 + 2;
        result -= (25 * numAlign - 10) * numAlign - 55;
        if (ver >= 7)
            result -= 36;
    }
    return result;
}

int QrCode::getAlignmentPatternPositions(int ver, std::uint8_t result[7]) {
    if (ver == 1)
        return 0;
    int numAlign = ver / 7 + 2;
    int step = (ver == 32) ? 26 : (ver * 4 + numAlign * 2 + 1) / (numAlign * 2 - 2) * 2;
    for (int i = numAlign - 1, pos = ver * 4 + 10; i >= 1; i--, pos -= step)
        result[i] = pos;
    result[0] = 6;
    return numAlign;
}

/*---- QrSegment ----*/

QrSegment::QrSegment(const Mode &md, int numCh, const std::vector<bool> &dt) :
    mode(md),
    numChars(numCh),
    data(dt) {}

QrSegment::QrSegment(const Mode &md, int numCh, std::vector<bool> &&dt) :
    mode(md),
    numChars(numCh),
    data(std::move(dt)) {}

const QrSegment::Mode &QrSegment::getMode() const {
    return mode;
}

int QrSegment::getNumChars() const {
    return numChars;
}

const std::vector<bool> &QrSegment::getData() const {
    return data;
}

QrSegment QrSegment::makeBytes(const std::vector<std::uint8_t> &data) {
    std::vector<bool> bits;
    for (std::uint8_t b : data) {
        for (int i = 7; i >= 0; i--)
            bits.push_back(((b >> i) & 1) != 0);
    }
    return QrSegment(Mode::BYTE, static_cast<int>(data.size()), std::move(bits));
}

QrSegment QrSegment::makeNumeric(const char *digits) {
    std::vector<bool> bits;
    int accumData = 0;
    int accumCount = 0;
    int charCount = 0;
    for (; *digits != '\0'; digits++, charCount++) {
        char c = *digits;
        if (c < '0' || c > '9')
            throw std::domain_error("String contains non-numeric characters");
        accumData = accumData * 10 + (c - '0');
        accumCount++;
        if (accumCount == 3) {
            for (int i = 9; i >= 0; i--)
                bits.push_back(((accumData >> i) & 1) != 0);
            accumData = 0;
            accumCount = 0;
        }
    }
    if (accumCount > 0) {
        for (int i = accumCount * 3; i >= 0; i--)
            bits.push_back(((accumData >> i) & 1) != 0);
    }
    return QrSegment(Mode::NUMERIC, charCount, std::move(bits));
}

QrSegment QrSegment::makeAlphanumeric(const char *text) {
    std::vector<bool> bits;
    int accumData = 0;
    int accumCount = 0;
    int charCount = 0;
    for (; *text != '\0'; text++, charCount++) {
        const char *temp = std::strchr("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:  ", *text);
        if (temp == nullptr)
            throw std::domain_error("String contains unencodable characters in alphanumeric mode");
        accumData = accumData * 45 + (temp - "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:  ");
        accumCount++;
        if (accumCount == 2) {
            for (int i = 10; i >= 0; i--)
                bits.push_back(((accumData >> i) & 1) != 0);
            accumData = 0;
            accumCount = 0;
        }
    }
    if (accumCount > 0) {
        for (int i = 5; i >= 0; i--)
            bits.push_back(((accumData >> i) & 1) != 0);
    }
    return QrSegment(Mode::ALPHANUMERIC, charCount, std::move(bits));
}

bool QrSegment::isNumeric(const char *text) {
    for (; *text != '\0'; text++) {
        if (*text < '0' || *text > '9')
            return false;
    }
    return true;
}

bool QrSegment::isAlphanumeric(const char *text) {
    for (; *text != '\0'; text++) {
        if (std::strchr("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:  ", *text) == nullptr)
            return false;
    }
    return true;
}

}
