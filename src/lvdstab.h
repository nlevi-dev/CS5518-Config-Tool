#pragma once
#include <QWidget>
#include "regbuilder.h"

QT_BEGIN_NAMESPACE
class QRadioButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
class QGroupBox;
class QPushButton;
QT_END_NAMESPACE

// "LVDS" tab: timing parameters, port count, colour depth, data format,
// per-port line routing, P/N swap, power-down, and advanced signal options.
class LvdsTab : public QWidget {
    Q_OBJECT
public:
    explicit LvdsTab(QWidget *parent = nullptr);

    void setDualPortVisible(bool visible);

    TimingParams  timingParams()  const;
    LvdsPortCfg   port0Config()   const;
    LvdsPortCfg   port1Config()   const;
    bool          dualPort()      const;
    bool          eightBit()      const;
    LvdsFormat    lvdsFormat()    const;
    AdvancedCfg   advancedCfg()   const;

signals:
    void timingChanged();

private slots:
    void onResolutionSelected(int index);
    void onRecalcPixelClock();
    void onPort0EnableToggled(bool on);
    void onPort1EnableToggled(bool on);

private:
    void recalcRefreshRate();
    void applyResolutionPreset(int hActive, int vActive);
    LvdsPortCfg readPortCfg(int port) const;

    // Timing
    QComboBox  *m_cmbResolution;
    QLineEdit  *m_edPixelClk;
    QLineEdit  *m_edRefreshRate;
    QLineEdit  *m_edHActive;
    QLineEdit  *m_edHBlank;
    QLineEdit  *m_edHFrontPorch;
    QLineEdit  *m_edHSyncWidth;
    QLineEdit  *m_edVActive;
    QLineEdit  *m_edVBlank;
    QLineEdit  *m_edVFrontPorch;
    QLineEdit  *m_edVSyncWidth;

    // Basic options
    QRadioButton *m_rbSinglePort;
    QRadioButton *m_rbDualPort;
    QRadioButton *m_rb6bit;
    QRadioButton *m_rb8bit;
    QRadioButton *m_rbSpwg;
    QRadioButton *m_rbOpenLdi;

    // Port 0 routing (5 channels: Line0–Line3, CLK)
    QComboBox  *m_cmbP0Route[5];
    QCheckBox  *m_cbP0PwrDn[5];
    QCheckBox  *m_cbP0PnSwap[5];
    QPushButton *m_btnP0Enable;
    QPushButton *m_btnP0Disable;

    // Port 1 routing
    QComboBox  *m_cmbP1Route[5];
    QCheckBox  *m_cbP1PwrDn[5];
    QCheckBox  *m_cbP1PnSwap[5];
    QPushButton *m_btnP1Enable;
    QPushButton *m_btnP1Disable;
    QGroupBox  *m_gbPort1;

    // Advanced
    QCheckBox  *m_cbInvVsync;
    QCheckBox  *m_cbInvHsync;
    QCheckBox  *m_cbInvData;
    QCheckBox  *m_cbInvClk;
    QCheckBox  *m_cbSwapOddEven;
    QCheckBox  *m_cbSwapPort01;
    QCheckBox  *m_cbDeOnly;
};
