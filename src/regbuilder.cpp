#include "regbuilder.h"
#include <QStringList>
#include <cmath>

// ---- helpers ---------------------------------------------------------------

static int b2i(bool v, int shift) { return (v ? 1 : 0) << shift; }

static QString hex(uint8_t v) {
    return QString("%1").arg(v, 2, 16, QChar('0')).toUpper();
}

// Encode a chip-side channel name to the low-nibble routing value (port 0 table).
static uint8_t routeLow0(int ch) {
    // ch: 0=Line0, 1=Line1, 2=Line2, 3=Line3, 4=CLK
    static const uint8_t tbl[] = {0x00, 0x01, 0x02, 0x04, 0x03};
    return (ch < 5) ? tbl[ch] : 0;
}

// Encode a chip-side channel name to the high-nibble routing value (port 0 table).
static uint8_t routeHigh0(int ch) {
    static const uint8_t tbl[] = {0x00, 0x10, 0x20, 0x40, 0x30};
    return (ch < 5) ? tbl[ch] : 0;
}

// Encode a chip-side channel name to the low-nibble routing value (port 1 table).
static uint8_t routeLow1(int ch) {
    static const uint8_t tbl[] = {0x05, 0x06, 0x07, 0x09, 0x08};
    return (ch < 5) ? tbl[ch] : 0;
}

// Encode a chip-side channel name to the high-nibble routing value (port 1 table).
static uint8_t routeHigh1(int ch) {
    static const uint8_t tbl[] = {0x50, 0x60, 0x70, 0x90, 0x80};
    return (ch < 5) ? tbl[ch] : 0;
}

// Encode MIPI lane count to the 0x05:0x0A register value.
static uint8_t encodeLaneCount(int lanes) {
    static const uint8_t tbl[] = {0x00, 0x20, 0x40, 0x60, 0x80};
    return (lanes >= 1 && lanes <= 4) ? tbl[lanes] : 0x80;
}

// The 24 valid MIPI P/N permutations and their register codes.
static const char *kPermStrings[24] = {
    "0123","0132","0213","0231","0312","0321",
    "1023","1032","1203","1230","1302","1320",
    "2013","2031","2103","2130","2301","2310",
    "3012","3021","3102","3120","3201","3210"
};
static const char *kPermCodes[24] = {
    "40","42","44","46","48","4A","4C","4E","50","52",
    "54","56","58","5A","5C","5E","60","62","64","66",
    "68","6A","6C","6E"
};

// Validate the MIPI lane routing permutation and return the register hex string.
// Returns empty string on invalid input.
static QString lookupLanePermCode(const int map[4]) {
    char key[5];
    for (int i = 0; i < 4; ++i) key[i] = '0' + map[i];
    key[4] = '\0';
    for (int i = 0; i < 24; ++i)
        if (strcmp(key, kPermStrings[i]) == 0)
            return QString(kPermCodes[i]);
    return {};
}

// ---- main builder ----------------------------------------------------------

QString buildRegTable(const BuildParams &p) {
    // We build two parallel strings: human-readable and raw command format.
    QString readable, raw;

    auto emit = [&](uint8_t page, uint8_t reg, uint8_t val) {
        readable += QString("0x%1 : 0x%2 = 0x%3 \r\n").arg(hex(page), hex(reg), hex(val));
        raw      += QString("\\W %1 %2 %3 \r\n").arg(hex(page), hex(reg), hex(val));
    };
    auto emitFixed = [&](uint8_t page, uint8_t reg, uint8_t val) {
        emit(page, reg, val);
    };

    // --- Fixed preamble ---
    emitFixed(0x10, 0x66, 0x05);
    emitFixed(0x0F, 0x00, 0x01);
    emitFixed(0x05, 0x2C, 0x00);
    emitFixed(0x10, 0xD8, 0x01);
    emitFixed(0x0F, 0x12, 0x00);
    emitFixed(0x05, 0x0C, 0x03);

    // --- MIPI lane count ---
    uint8_t regLane = encodeLaneCount(p.mipi.laneCount);
    emit(0x05, 0x0A, regLane);

    // --- PLL calculation ---
    double pixClk = p.timing.pixelClkMhz;
    double mipiClk = p.mipi.mipiClkMhz;

    // Determine reference clock
    double refClk = mipiClk / 4.0;
    {
        double div = mipiClk / 4.0 / 50.0;
        int n = (int)std::ceil(div);
        if (n < 1) n = 1;
        refClk = mipiClk / (4.0 * n);
    }

    // Clock source register
    if (p.clkSrc == ClkSource::External) {
        refClk = p.extClkMhz;
        emitFixed(0x0F, 0x25, 0x01);
    } else if (p.clkSrc == ClkSource::MipiDerived) {
        double div = mipiClk / 4.0 / 50.0;
        int n = (int)std::ceil(div);
        if (n < 1) n = 1;
        uint32_t nReg = static_cast<uint32_t>(n) | 0x10;
        emit(0x0F, 0x25, static_cast<uint8_t>(nReg));
    } else { // Internal27M
        refClk = 27.0;
        emitFixed(0x0F, 0x25, 0x21);
    }

    // H multiplier selection
    double effPixClk = p.dualPort ? (pixClk / 2.0) : pixClk;
    uint8_t hReg = 0;
    double pllOut = 0.0;
    int hMul = 1;
    for (int i = 0; i < 4; ++i) {
        pllOut = effPixClk * hMul * 7.0;
        if (pllOut >= 700.0 && pllOut <= 1400.0) {
            if (hMul == 2) hReg |= 0x04;
            else if (hMul == 4) hReg |= 0x08;
            else if (hMul == 8) hReg |= 0x0C;
            break;
        }
        hMul *= 2;
    }
    emit(0x0F, 0x21, hReg);

    // PLL integer/fractional divider
    double ratio = pllOut / refClk;
    double intPart = std::floor(ratio);
    int    intDiv  = (int)intPart;
    double fracPart = ratio - intPart;
    fracPart = std::round(fracPart * 1000.0) / 1000.0;
    if (fracPart >= 1.0) { intDiv++; fracPart = 0.0; }
    uint32_t fracReg = static_cast<uint32_t>(fracPart * 4294967296.0);
    emit(0x10, 0x06, static_cast<uint8_t>(intDiv));
    emit(0x10, 0x07, static_cast<uint8_t>((fracReg >> 24) & 0xFF));
    emit(0x10, 0x08, static_cast<uint8_t>((fracReg >> 16) & 0xFF));
    emit(0x10, 0x09, static_cast<uint8_t>((fracReg >>  8) & 0xFF));

    // --- Fixed PLL enable sequence ---
    emitFixed(0x10, 0x01, 0x03);
    emitFixed(0x10, 0x03, 0x00);
    emitFixed(0x10, 0x00, 0x18);
    emitFixed(0x10, 0x0D, 0x05);

    // --- LVDS port P/N swap and power-down ---
    uint8_t r01 = static_cast<uint8_t>(
        b2i(p.port0.powerDown[3], 0) |
        b2i(p.port0.powerDown[4], 1) |
        0x40);
    uint8_t r02, r03, r04;
    if (p.dualPort) {
        if (r01 != 0x43) emit(0x0F, 0x01, r01);
        r02 = static_cast<uint8_t>(
            b2i(p.port0.powerDown[0], 0) | b2i(p.port0.powerDown[1], 1) |
            b2i(p.port0.powerDown[2], 2) | b2i(p.port0.powerDown[3], 3) |
            b2i(p.port0.powerDown[4], 4) | b2i(p.port1.powerDown[2], 5) |
            b2i(p.port1.powerDown[1], 6) | b2i(p.port1.powerDown[0], 7));
        r03 = static_cast<uint8_t>(
            b2i(p.port1.powerDown[3], 0) |
            b2i(p.port1.powerDown[4], 1) |
            0x40);
        r04 = static_cast<uint8_t>(
            b2i(p.port0.pnSwap[0], 0) | b2i(p.port0.pnSwap[1], 1) |
            b2i(p.port0.pnSwap[2], 2) | b2i(p.port0.pnSwap[3], 3) |
            b2i(p.port0.pnSwap[4], 4) | b2i(p.port1.pnSwap[2], 5) |
            b2i(p.port1.pnSwap[1], 6) | b2i(p.port1.pnSwap[0], 7));
        if (r02 != 0xFF) emit(0x0F, 0x02, r02);
        if (r03 != 0x40) emit(0x0F, 0x03, r03);
        if (r04 != 0x00) emit(0x0F, 0x04, r04);
    } else {
        r02 = static_cast<uint8_t>(
            b2i(p.port0.powerDown[0], 0) | b2i(p.port0.powerDown[1], 1) |
            b2i(p.port0.powerDown[2], 2) | b2i(p.port0.powerDown[3], 3) |
            b2i(p.port0.powerDown[4], 4));
        r04 = static_cast<uint8_t>(
            b2i(p.port0.pnSwap[0], 0) | b2i(p.port0.pnSwap[1], 1) |
            b2i(p.port0.pnSwap[2], 2) | b2i(p.port0.pnSwap[3], 3) |
            b2i(p.port0.pnSwap[4], 4));
        emitFixed(0x0F, 0x01, 0x40);
        emit(0x0F, 0x02, r02);
        emitFixed(0x0F, 0x03, 0x40);
        emit(0x0F, 0x04, r04);
    }

    // --- LVDS output config register 0x2C:0x00 ---
    uint8_t r2c00 = static_cast<uint8_t>(
        1 |
        b2i(p.dualPort,          1) |
        b2i(!p.eightBit,         2) |
        b2i(p.lvdsFmt == LvdsFormat::OpenLdi, 3) |
        b2i(p.adv.swapPort01,    4) |
        b2i(p.adv.invertClk,     5) |
        b2i(p.adv.invertData,    6) |
        b2i(p.adv.deOnlyMode,    7));
    if (r2c00 != 0x05) emit(0x2C, 0x00, r2c00);

    uint8_t r2c01 = static_cast<uint8_t>(
        b2i(p.adv.invertVsync, 7) |
        b2i(p.adv.invertHsync, 6));
    if (r2c01 != 0x00) emit(0x2C, 0x01, r2c01);

    emitFixed(0x2C, 0x02, 0x01);

    // --- Port 0 line routing (0x2C:0x0A–0x0C) ---
    // 0x0A: [line0→panel, line1→panel]
    uint8_t r2c0a = routeLow0(p.port0.lineMap[0]) | routeHigh0(p.port0.lineMap[1]);
    if (r2c0a != 0x10) emit(0x2C, 0x0A, r2c0a);
    // 0x0B: [line2→panel, line3→panel]
    uint8_t r2c0b = routeLow0(p.port0.lineMap[2]) | routeHigh0(p.port0.lineMap[3]);
    if (r2c0b != 0x42) emit(0x2C, 0x0B, r2c0b);
    // 0x0C: CLK routing
    uint8_t r2c0c = routeLow0(p.port0.lineMap[4]);
    if (r2c0c != 0x03) emit(0x2C, 0x0C, r2c0c);

    // --- Port 1 line routing (0x2C:0x0D–0x0F) ---
    uint8_t r2c0d = routeLow1(p.port1.lineMap[0]) | routeHigh1(p.port1.lineMap[1]);
    if (r2c0d != 0x65) emit(0x2C, 0x0D, r2c0d);
    uint8_t r2c0e = routeLow1(p.port1.lineMap[2]) | routeHigh1(p.port1.lineMap[3]);
    if (r2c0e != 0x87) emit(0x2C, 0x0E, r2c0e);
    uint8_t r2c0f = routeLow1(p.port1.lineMap[4]);
    if (r2c0f != 0x09) emit(0x2C, 0x0F, r2c0f);

    // --- Horizontal active (half-pixel for dual-port) ---
    int hHalf = p.timing.hActive / 2;
    emit(0x05, 0x1C, static_cast<uint8_t>((hHalf >> 8) & 0xFF));
    emit(0x05, 0x1D, static_cast<uint8_t>(hHalf & 0xFF));

    // --- Fixed output config ---
    emitFixed(0x05, 0x0B, 0x10);
    emitFixed(0x05, 0x24, 0x02);
    emitFixed(0x05, 0x28, 0x04);
    emitFixed(0x0F, 0x10, 0x44);
    emitFixed(0x0F, 0x11, 0x44);
    emitFixed(0x0F, 0x18, 0x60);

    // --- Timing registers ---
    int va = p.timing.vActive;
    emit(0x10, 0x81, static_cast<uint8_t>(va & 0xFF));
    emit(0x10, 0x82, static_cast<uint8_t>((va >> 8) & 0xFF));

    int haHalf = p.timing.hActive / 2;
    emit(0x10, 0x83, static_cast<uint8_t>(haHalf & 0xFF));
    emit(0x10, 0x84, static_cast<uint8_t>((haHalf >> 8) & 0xFF));

    int htHalf = (p.timing.hActive + p.timing.hBlank) / 2;
    emit(0x10, 0x85, static_cast<uint8_t>(htHalf & 0xFF));
    emit(0x10, 0x86, static_cast<uint8_t>((htHalf >> 8) & 0xFF));

    int hfpHalf = p.timing.hFrontPorch / 2;
    emit(0x10, 0x87, static_cast<uint8_t>(hfpHalf & 0xFF));
    emit(0x10, 0x88, static_cast<uint8_t>((hfpHalf >> 8) & 0xFF));

    int hswHalf = p.timing.hSyncWidth / 2;
    emit(0x10, 0x89, static_cast<uint8_t>(hswHalf & 0xFF));
    emit(0x10, 0x8A, static_cast<uint8_t>((hswHalf >> 8) & 0xFF));

    int hbpHalf = p.timing.hBlank / 2 - hfpHalf - hswHalf;
    emit(0x10, 0x8B, static_cast<uint8_t>(hbpHalf & 0xFF));
    emit(0x10, 0x8C, static_cast<uint8_t>((hbpHalf >> 8) & 0xFF));

    emit(0x10, 0x8D, static_cast<uint8_t>(p.timing.vFrontPorch));
    emit(0x10, 0x8E, static_cast<uint8_t>(p.timing.vSyncWidth));

    int vbp = p.timing.vBlank - p.timing.vFrontPorch - p.timing.vSyncWidth;
    emit(0x10, 0x8F, static_cast<uint8_t>(vbp));

    // --- BIST ---
    uint8_t bistReg = static_cast<uint8_t>(
        b2i(p.bistEnable, 0) |
        b2i(p.bistEnable && p.bistPat == BistPattern::GrayBar,   1) |
        b2i(p.bistEnable && p.bistPat == BistPattern::MobileBar, 2));
    emit(0x10, 0x80, bistReg);

    // --- Fixed tail ---
    emitFixed(0x10, 0x22, 0xDB);
    emitFixed(0x10, 0x22, 0xFD);
    emitFixed(0x10, 0x70, 0x00);
    emitFixed(0x10, 0x72, 0x4C);

    return (p.cmdFmt == CmdFormat::MipiLpCmd) ? readable : raw;
}
