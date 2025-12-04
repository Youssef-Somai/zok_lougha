/*
 * QR Code generator library (C++)
 *
 * Copyright (c) Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/qr-code-generator-library
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 */

#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace qrcodegen {

class QrCode final {

public:
    enum class Ecc {
        LOW = 0,
        MEDIUM = 1,
        QUARTILE = 2,
        HIGH = 3,
    };

public:
    static QrCode encodeText(const char *text, Ecc ecl);
    static QrCode encodeBinary(const std::vector<std::uint8_t> &data, Ecc ecl);

public:
    int getSize() const;
    bool getModule(int x, int y) const;

private:
    int size;
    int errorCorrectionLevel;
    std::vector<bool> modules;
    std::vector<bool> isFunction;

    QrCode(int ver, Ecc ecl);

    void drawFunctionPatterns();
    void drawFormatBits(int mask);
    void drawVersion();
    void drawFinderPattern(int x, int y);
    void drawAlignmentPattern(int x, int y);
    void setFunctionModule(int x, int y, bool isDark);
    void setModule(int x, int y, bool isDark);

    void appendErrorCorrection(std::vector<std::uint8_t> &data);
    void drawCodewords(const std::vector<std::uint8_t> &data);
    void applyMask(int mask);
    int getPenaltyScore() const;

    static int getAlignmentPatternPositions(int ver, std::uint8_t result[7]);
    static int getNumDataCodewords(int ver, Ecc ecl);
    static int getNumRawDataModules(int ver);

    friend class QrSegment;
};


class QrSegment final {
public:
    struct Mode {
        static const Mode NUMERIC;
        static const Mode ALPHANUMERIC;
        static const Mode BYTE;
        static const Mode KANJI;
        static const Mode ECI;

    private:
        int modeBits;
        int numBitsCharCount[3];
        explicit Mode(int mode, int cc0, int cc1, int cc2);
    public:
        int getModeBits() const;
        int numCharCountBits(int ver) const;
    };

private:
    Mode mode;
    int numChars;
    std::vector<bool> data;

public:
    static QrSegment makeBytes(const std::vector<std::uint8_t> &data);
    static QrSegment makeNumeric(const char *digits);
    static QrSegment makeAlphanumeric(const char *text);

public:
    QrSegment(const Mode &md, int numCh, const std::vector<bool> &dt);
    QrSegment(const Mode &md, int numCh, std::vector<bool> &&dt);

    const Mode &getMode() const;
    int getNumChars() const;
    const std::vector<bool> &getData() const;

    static bool isNumeric(const char *text);
    static bool isAlphanumeric(const char *text);
};

}
