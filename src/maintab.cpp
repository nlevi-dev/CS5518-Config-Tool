#include "maintab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>

MainTab::MainTab(QWidget *parent) : QWidget(parent) {
    // --- Chip variant ---
    auto *gbChip = new QGroupBox("Chip Model");
    m_rbCS5518 = new QRadioButton("CS5518 (48-pin, dual-port LVDS)");
    m_rbCS5516 = new QRadioButton("CS5516 (40-pin, single-port LVDS)");
    m_rbCS5518->setChecked(true);
    auto *hChip = new QHBoxLayout;
    hChip->addWidget(m_rbCS5518);
    hChip->addWidget(m_rbCS5516);
    gbChip->setLayout(hChip);

    // --- Command format ---
    auto *gbCmd = new QGroupBox("Output Command Format");
    m_rbMipiCmd = new QRadioButton("MIPI LP command");
    m_rbIicCmd  = new QRadioButton("IIC");
    m_rbMipiCmd->setChecked(true);
    auto *hCmd = new QHBoxLayout;
    hCmd->addWidget(m_rbMipiCmd);
    hCmd->addWidget(m_rbIicCmd);
    gbCmd->setLayout(hCmd);

    // --- BIST test mode ---
    auto *gbBist = new QGroupBox("Built-in Self-Test (BIST)");
    m_cbBistEnable = new QCheckBox("Enable BIST");
    m_rbColorBar   = new QRadioButton("Color Bar");
    m_rbGrayBar    = new QRadioButton("Gray Bar");
    m_rbMobileBar  = new QRadioButton("Mobile Bar");
    m_rbColorBar->setChecked(true);
    m_rbColorBar->setEnabled(false);
    m_rbGrayBar->setEnabled(false);
    m_rbMobileBar->setEnabled(false);

    auto *gbBpc = new QGroupBox("BPC");
    m_rb8bpc = new QRadioButton("8 bpc");
    m_rb6bpc = new QRadioButton("6 bpc");
    m_rb8bpc->setChecked(true);
    m_rb8bpc->setEnabled(false);
    m_rb6bpc->setEnabled(false);
    auto *hBpc = new QHBoxLayout;
    hBpc->addWidget(m_rb8bpc);
    hBpc->addWidget(m_rb6bpc);
    gbBpc->setLayout(hBpc);
    gbBpc->setVisible(false);

    auto *hPat = new QHBoxLayout;
    hPat->addWidget(m_rbColorBar);
    hPat->addWidget(m_rbGrayBar);
    hPat->addWidget(m_rbMobileBar);

    auto *vBist = new QVBoxLayout;
    vBist->addWidget(m_cbBistEnable);
    vBist->addLayout(hPat);
    vBist->addWidget(gbBpc);
    gbBist->setLayout(vBist);

    connect(m_cbBistEnable, &QCheckBox::toggled, this, [this](bool on) {
        m_rbColorBar->setEnabled(on);
        m_rbGrayBar->setEnabled(on);
        m_rbMobileBar->setEnabled(on);
    });
    connect(m_rbCS5518, &QRadioButton::toggled, this, &MainTab::onChipChanged);
    connect(m_rbCS5516, &QRadioButton::toggled, this, &MainTab::onChipChanged);

    auto *root = new QVBoxLayout(this);
    root->addWidget(gbChip);
    root->addWidget(gbCmd);
    root->addWidget(gbBist);
    root->addStretch();
}

void MainTab::onChipChanged() {
    emit chipVariantChanged(chipVariant());
}

ChipVariant MainTab::chipVariant() const {
    return m_rbCS5518->isChecked() ? ChipVariant::CS5518 : ChipVariant::CS5516;
}

CmdFormat MainTab::cmdFormat() const {
    return m_rbMipiCmd->isChecked() ? CmdFormat::MipiLpCmd : CmdFormat::Iic;
}

bool MainTab::bistEnabled() const {
    return m_cbBistEnable->isChecked();
}

BistPattern MainTab::bistPattern() const {
    if (m_rbGrayBar->isChecked())   return BistPattern::GrayBar;
    if (m_rbMobileBar->isChecked()) return BistPattern::MobileBar;
    return BistPattern::ColorBar;
}
