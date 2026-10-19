#pragma once
#include <QString>
#include <cstdint>

// Chip variant — determines which port-2 registers are available.
enum class ChipVariant { CS5518, CS5516 };

// Output command format.
enum class CmdFormat { MipiLpCmd, Iic };

// Reference clock source for the PLL.
enum class ClkSource { MipiDerived, External, Internal27M };

// LVDS data encoding standard.
enum class LvdsFormat { Spwg, OpenLdi };

// BIST (built-in self-test) pattern.
enum class BistPattern { ColorBar, GrayBar, MobileBar };

struct TimingParams {
    double pixelClkMhz = 148.5;
    int    hActive      = 1920;
    int    hBlank       = 280;
    int    hFrontPorch  = 5;
    int    hSyncWidth   = 5;
    int    vActive      = 1080;
    int    vBlank       = 45;
    int    vFrontPorch  = 10;
    int    vSyncWidth   = 5;
};

struct LvdsPortCfg {
    // line routing: index = chip channel (0=Line0..3=Line3, 4=CLK),
    //               value = panel channel
    int    lineMap[5]  = {0,1,2,3,4};  // default identity
    bool   pnSwap[5]   = {};
    bool   powerDown[5]= {};
};

struct MipiCfg {
    int    laneCount    = 4;           // 1–4
    double mipiClkMhz   = 445.5;
    // P/N swap per lane (0–3)
    bool   laneSwap[4]  = {};
    // lane routing: index = chip lane, value = panel lane
    int    laneMap[4]   = {0,1,2,3};
};

struct AdvancedCfg {
    bool   invertVsync  = false;
    bool   invertHsync  = false;
    bool   invertData   = false;
    bool   invertClk    = false;
    bool   swapOddEven  = false;
    bool   deOnlyMode   = true;   // force VSYNC/HSYNC=0
    bool   swapPort01   = false;
};

struct BuildParams {
    ChipVariant  variant     = ChipVariant::CS5518;
    CmdFormat    cmdFmt      = CmdFormat::MipiLpCmd;
    ClkSource    clkSrc      = ClkSource::MipiDerived;
    double       extClkMhz   = 26.0;   // used when clkSrc == External
    LvdsFormat   lvdsFmt     = LvdsFormat::Spwg;
    bool         dualPort    = true;
    bool         eightBit    = true;
    bool         bistEnable  = false;
    BistPattern  bistPat     = BistPattern::ColorBar;
    TimingParams timing;
    LvdsPortCfg  port0;
    LvdsPortCfg  port1;
    MipiCfg      mipi;
    AdvancedCfg  adv;
};

// Builds the register table string from the given parameters.
// Returns the formatted output in the requested CmdFormat.
QString buildRegTable(const BuildParams &p);

