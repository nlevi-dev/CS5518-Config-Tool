#include "lvdstab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <cmath>

static const QStringList kResolutionItems = {
    "Set Manually",
    "1920x1200","1920x1080","1680x1050","1600x1200","1600x900",
    "1440x1050","1440x900","1366x768","1280x1024","1280x960",
    "1280x800","1280x768","1280x720","1024x768","1024x600","800x600"
};

static const QStringList kChannelItems = {"Line0","Line1","Line2","Line3","CLK"};

static QGroupBox *buildPortGroup(const QString &title,
    QComboBox *routes[5], QCheckBox *pwrDn[5], QCheckBox *pnSwap[5],
    QPushButton *&btnEn, QPushButton *&btnDis)
{
    auto *gb = new QGroupBox(title);
    static const char *kNames[] = {"Line0","Line1","Line2","Line3","CLK"};

    auto *colName  = new QVBoxLayout;
    auto *colRoute = new QVBoxLayout;
    auto *colPwr   = new QVBoxLayout;
    auto *colPn    = new QVBoxLayout;
    colName->addWidget(new QLabel("Channel"));
    colRoute->addWidget(new QLabel("Routing"));
    colPwr->addWidget(new QLabel("Power Down"));
    colPn->addWidget(new QLabel("P/N Swap"));

    for (int i = 0; i < 5; ++i) {
        colName->addWidget(new QLabel(kNames[i]));
        routes[i] = new QComboBox;
        routes[i]->addItems(kChannelItems);
        routes[i]->setCurrentIndex(i);
        colRoute->addWidget(routes[i]);
        pwrDn[i] = new QCheckBox;
        colPwr->addWidget(pwrDn[i]);
        pnSwap[i] = new QCheckBox;
        colPn->addWidget(pnSwap[i]);
    }

    btnEn  = new QPushButton("Enable all");
    btnDis = new QPushButton("Disable all");
    auto *hBtn = new QHBoxLayout;
    hBtn->addWidget(btnEn);
    hBtn->addWidget(btnDis);

    auto *hGrid = new QHBoxLayout;
    hGrid->addLayout(colName);
    hGrid->addLayout(colRoute);
    hGrid->addLayout(colPwr);
    hGrid->addLayout(colPn);

    auto *vMain = new QVBoxLayout;
    vMain->addLayout(hGrid);
    vMain->addLayout(hBtn);
    gb->setLayout(vMain);
    return gb;
}

LvdsTab::LvdsTab(QWidget *parent) : QWidget(parent) {
    // --- Timing ---
    auto *gbTiming = new QGroupBox("Timing Parameters");
    m_cmbResolution = new QComboBox;
    m_cmbResolution->addItems(kResolutionItems);
    m_cmbResolution->setCurrentText("1920x1080");

    auto makeField = [](QLineEdit *&ed, const QString &def) {
        ed = new QLineEdit(def);
        ed->setFixedWidth(80);
        ed->setAlignment(Qt::AlignRight);
    };
    makeField(m_edPixelClk,    "148.50");
    makeField(m_edRefreshRate, "60.00");
    makeField(m_edHActive,     "1920");
    makeField(m_edHBlank,      "280");
    makeField(m_edHFrontPorch, "5");
    makeField(m_edHSyncWidth,  "5");
    makeField(m_edVActive,     "1080");
    makeField(m_edVBlank,      "31");
    makeField(m_edVFrontPorch, "10");
    makeField(m_edVSyncWidth,  "5");

    auto *frmTiming = new QFormLayout;
    auto *hRes = new QHBoxLayout;
    hRes->addWidget(m_cmbResolution);
    hRes->addStretch();
    frmTiming->addRow("Resolution preset:", hRes);
    auto *hClk = new QHBoxLayout;
    hClk->addWidget(m_edPixelClk); hClk->addWidget(new QLabel("MHz"));
    frmTiming->addRow("Pixel clock:", hClk);
    auto *hRr = new QHBoxLayout;
    hRr->addWidget(m_edRefreshRate); hRr->addWidget(new QLabel("Hz"));
    frmTiming->addRow("Refresh rate:", hRr);
    frmTiming->addRow("H Active:",      m_edHActive);
    frmTiming->addRow("H Blank:",       m_edHBlank);
    frmTiming->addRow("H Front Porch:", m_edHFrontPorch);
    frmTiming->addRow("H Sync Width:",  m_edHSyncWidth);
    frmTiming->addRow("V Active:",      m_edVActive);
    frmTiming->addRow("V Blank:",       m_edVBlank);
    frmTiming->addRow("V Front Porch:", m_edVFrontPorch);
    frmTiming->addRow("V Sync Width:",  m_edVSyncWidth);
    gbTiming->setLayout(frmTiming);

    // --- Basic options ---
    auto *gbPorts = new QGroupBox("Port Count");
    m_rbSinglePort = new QRadioButton("Single port");
    m_rbDualPort   = new QRadioButton("Dual port");
    m_rbDualPort->setChecked(true);
    auto *hPorts = new QHBoxLayout;
    hPorts->addWidget(m_rbSinglePort); hPorts->addWidget(m_rbDualPort);
    gbPorts->setLayout(hPorts);

    auto *gbDepth = new QGroupBox("Colour Depth");
    m_rb6bit = new QRadioButton("6-bit");
    m_rb8bit = new QRadioButton("8-bit");
    m_rb8bit->setChecked(true);
    auto *hDepth = new QHBoxLayout;
    hDepth->addWidget(m_rb6bit); hDepth->addWidget(m_rb8bit);
    gbDepth->setLayout(hDepth);

    auto *gbFmt = new QGroupBox("LVDS Data Format");
    m_rbSpwg    = new QRadioButton("SPWG (VESA) [common]");
    m_rbOpenLdi = new QRadioButton("OPENLDI (JEIDA)");
    m_rbSpwg->setChecked(true);
    auto *hFmt = new QHBoxLayout;
    hFmt->addWidget(m_rbSpwg); hFmt->addWidget(m_rbOpenLdi);
    gbFmt->setLayout(hFmt);

    auto *hBasic = new QHBoxLayout;
    hBasic->addWidget(gbPorts); hBasic->addWidget(gbDepth); hBasic->addWidget(gbFmt);

    // --- Port routing groups ---
    auto *gbPort0 = buildPortGroup("PORT 0 — Line Routing / P/N Swap",
        m_cmbP0Route, m_cbP0PwrDn, m_cbP0PnSwap, m_btnP0Enable, m_btnP0Disable);

    m_gbPort1 = buildPortGroup("PORT 1 — Line Routing / P/N Swap",
        m_cmbP1Route, m_cbP1PwrDn, m_cbP1PnSwap, m_btnP1Enable, m_btnP1Disable);

    // Default port 1 routing (matches original: Line0→0, Line1→1, Line2→2, Line3→CLK, CLK→Line3)
    m_cmbP1Route[0]->setCurrentIndex(0);
    m_cmbP1Route[1]->setCurrentIndex(1);
    m_cmbP1Route[2]->setCurrentIndex(2);
    m_cmbP1Route[3]->setCurrentIndex(4);
    m_cmbP1Route[4]->setCurrentIndex(3);

    // --- Advanced ---
    auto *gbAdv = new QGroupBox("Advanced Signal Options");
    m_cbInvVsync    = new QCheckBox("Invert VSYNC");
    m_cbInvHsync    = new QCheckBox("Invert HSYNC");
    m_cbInvData     = new QCheckBox("Invert data");
    m_cbInvClk      = new QCheckBox("Invert clock");
    m_cbSwapOddEven = new QCheckBox("Swap odd/even pixels");
    m_cbSwapPort01  = new QCheckBox("Swap PORT 0/1");
    m_cbDeOnly      = new QCheckBox("DE-only mode (force VSYNC/HSYNC=0)");
    m_cbDeOnly->setChecked(true);
    auto *hAdv = new QHBoxLayout;
    hAdv->addWidget(m_cbInvVsync); hAdv->addWidget(m_cbInvHsync);
    hAdv->addWidget(m_cbInvData);  hAdv->addWidget(m_cbInvClk);
    hAdv->addWidget(m_cbSwapOddEven); hAdv->addWidget(m_cbSwapPort01);
    hAdv->addWidget(m_cbDeOnly);
    gbAdv->setLayout(hAdv);

    // --- Root layout ---
    auto *hPorts2 = new QHBoxLayout;
    hPorts2->addWidget(gbPort0);
    hPorts2->addWidget(m_gbPort1);

    auto *root = new QVBoxLayout(this);
    root->addWidget(gbTiming);
    root->addLayout(hBasic);
    root->addLayout(hPorts2);
    root->addWidget(gbAdv);

    // --- Connections ---
    connect(m_cmbResolution, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LvdsTab::onResolutionSelected);
    connect(m_edRefreshRate, &QLineEdit::returnPressed, this, &LvdsTab::onRecalcPixelClock);
    connect(m_edHActive,     &QLineEdit::returnPressed, this, &LvdsTab::onRecalcPixelClock);
    connect(m_edHBlank,      &QLineEdit::returnPressed, this, &LvdsTab::onRecalcPixelClock);
    connect(m_edVActive,     &QLineEdit::returnPressed, this, &LvdsTab::onRecalcPixelClock);
    connect(m_edVBlank,      &QLineEdit::returnPressed, this, &LvdsTab::onRecalcPixelClock);
    connect(m_rbDualPort, &QRadioButton::toggled, this, [this](bool on) {
        m_gbPort1->setVisible(on);
        m_cbSwapPort01->setEnabled(on);
    });
    connect(m_btnP0Enable,  &QPushButton::clicked, this, [this]{ onPort0EnableToggled(true);  });
    connect(m_btnP0Disable, &QPushButton::clicked, this, [this]{ onPort0EnableToggled(false); });
    connect(m_btnP1Enable,  &QPushButton::clicked, this, [this]{ onPort1EnableToggled(true);  });
    connect(m_btnP1Disable, &QPushButton::clicked, this, [this]{ onPort1EnableToggled(false); });
}

void LvdsTab::setDualPortVisible(bool visible) {
    m_gbPort1->setVisible(visible);
    if (!visible) m_rbSinglePort->setChecked(true);
    m_rbDualPort->setEnabled(visible);
    m_cbSwapPort01->setEnabled(visible);
}

void LvdsTab::onResolutionSelected(int index) {
    if (index == 0) return;
    QString text = m_cmbResolution->currentText();
    QStringList parts = text.split('x');
    if (parts.size() != 2) return;
    applyResolutionPreset(parts[0].toInt(), parts[1].toInt());
}

void LvdsTab::applyResolutionPreset(int hActive, int vActive) {
    m_edHActive->setText(QString::number(hActive));
    m_edHBlank->setText("280");
    m_edHFrontPorch->setText("5");
    m_edHSyncWidth->setText("5");
    m_edVActive->setText(QString::number(vActive));

    // Integer division first (matching original C# integer arithmetic), then +1
    int vBlank = (276 * vActive / 9724) + 1;

    int vSync = 5; // default
    int ar4_3  = hActive * 3 / 4;
    int ar16_9 = hActive * 9 / 16;
    int ar16_10= hActive * 10 / 16;
    int ar5_4  = hActive * 4 / 5;
    if      (vActive == ar4_3)  vSync = 4;
    else if (vActive == ar16_9) vSync = 5;
    else if (vActive == ar16_10)vSync = 6;
    else if (vActive == ar5_4)  vSync = 7;

    m_edVBlank->setText(QString::number(vBlank));
    m_edVFrontPorch->setText("10");
    m_edVSyncWidth->setText(QString::number(vSync));
    m_edRefreshRate->setText("60");
    onRecalcPixelClock();
}

void LvdsTab::onRecalcPixelClock() {
    bool ok1, ok2, ok3, ok4, ok5;
    int ha = m_edHActive->text().toInt(&ok1);
    int hb = m_edHBlank->text().toInt(&ok2);
    int va = m_edVActive->text().toInt(&ok3);
    int vb = m_edVBlank->text().toInt(&ok4);
    double rr = m_edRefreshRate->text().toDouble(&ok5);
    if (!ok1||!ok2||!ok3||!ok4||!ok5) return;
    double pclk = (double)(ha + hb) * (va + vb) * rr / 1e6;
    m_edPixelClk->setText(QString::number(pclk, 'f', 2));
    emit timingChanged();
}

void LvdsTab::recalcRefreshRate() {
    bool ok1, ok2, ok3, ok4, ok5;
    double pclk = m_edPixelClk->text().toDouble(&ok1);
    int ha = m_edHActive->text().toInt(&ok2);
    int hb = m_edHBlank->text().toInt(&ok3);
    int va = m_edVActive->text().toInt(&ok4);
    int vb = m_edVBlank->text().toInt(&ok5);
    if (!ok1||!ok2||!ok3||!ok4||!ok5) return;
    double rr = pclk * 1e6 / ((ha + hb) * (va + vb));
    m_edRefreshRate->setText(QString::number(rr, 'f', 2));
}

void LvdsTab::onPort0EnableToggled(bool on) {
    for (int i = 0; i < 5; ++i) {
        m_cmbP0Route[i]->setEnabled(on);
        m_cbP0PwrDn[i]->setEnabled(on);
        m_cbP0PnSwap[i]->setEnabled(on);
    }
}

void LvdsTab::onPort1EnableToggled(bool on) {
    for (int i = 0; i < 5; ++i) {
        m_cmbP1Route[i]->setEnabled(on);
        m_cbP1PwrDn[i]->setEnabled(on);
        m_cbP1PnSwap[i]->setEnabled(on);
    }
}

LvdsPortCfg LvdsTab::readPortCfg(int port) const {
    LvdsPortCfg cfg;
    QComboBox **routes = (port == 0) ? const_cast<QComboBox**>(m_cmbP0Route)
                                     : const_cast<QComboBox**>(m_cmbP1Route);
    QCheckBox **pwrDn  = (port == 0) ? const_cast<QCheckBox**>(m_cbP0PwrDn)
                                     : const_cast<QCheckBox**>(m_cbP1PwrDn);
    QCheckBox **pnSwap = (port == 0) ? const_cast<QCheckBox**>(m_cbP0PnSwap)
                                     : const_cast<QCheckBox**>(m_cbP1PnSwap);
    for (int i = 0; i < 5; ++i) {
        cfg.lineMap[i]   = routes[i]->currentIndex();
        cfg.powerDown[i] = pwrDn[i]->isChecked();
        cfg.pnSwap[i]    = pnSwap[i]->isChecked();
    }
    return cfg;
}

TimingParams LvdsTab::timingParams() const {
    TimingParams t;
    t.pixelClkMhz  = m_edPixelClk->text().toDouble();
    t.hActive      = m_edHActive->text().toInt();
    t.hBlank       = m_edHBlank->text().toInt();
    t.hFrontPorch  = m_edHFrontPorch->text().toInt();
    t.hSyncWidth   = m_edHSyncWidth->text().toInt();
    t.vActive      = m_edVActive->text().toInt();
    t.vBlank       = m_edVBlank->text().toInt();
    t.vFrontPorch  = m_edVFrontPorch->text().toInt();
    t.vSyncWidth   = m_edVSyncWidth->text().toInt();
    return t;
}

LvdsPortCfg LvdsTab::port0Config() const { return readPortCfg(0); }
LvdsPortCfg LvdsTab::port1Config() const { return readPortCfg(1); }
bool        LvdsTab::dualPort()    const { return m_rbDualPort->isChecked(); }
bool        LvdsTab::eightBit()    const { return m_rb8bit->isChecked(); }
LvdsFormat  LvdsTab::lvdsFormat()  const {
    return m_rbOpenLdi->isChecked() ? LvdsFormat::OpenLdi : LvdsFormat::Spwg;
}
AdvancedCfg LvdsTab::advancedCfg() const {
    AdvancedCfg a;
    a.invertVsync  = m_cbInvVsync->isChecked();
    a.invertHsync  = m_cbInvHsync->isChecked();
    a.invertData   = m_cbInvData->isChecked();
    a.invertClk    = m_cbInvClk->isChecked();
    a.swapOddEven  = m_cbSwapOddEven->isChecked();
    a.swapPort01   = m_cbSwapPort01->isChecked();
    a.deOnlyMode   = m_cbDeOnly->isChecked();
    return a;
}
