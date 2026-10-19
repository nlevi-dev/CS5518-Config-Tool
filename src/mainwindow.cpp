#include "mainwindow.h"
#include "maintab.h"
#include "lvdstab.h"
#include "mipitab.h"
#include "regbuilder.h"

#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("CS5518/16 Register Generator");
    setMinimumSize(1300, 900);
    resize(1400, 950);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *tabs = new QTabWidget;
    m_tabMain = new MainTab;
    m_tabLvds = new LvdsTab;
    m_tabMipi = new MipiTab;
    tabs->addTab(m_tabMain, "MAIN");
    tabs->addTab(m_tabLvds, "LVDS");
    tabs->addTab(m_tabMipi, "MIPI");
    tabs->setCurrentIndex(0);

    m_edOutput = new QTextEdit;
    m_edOutput->setReadOnly(true);
    m_edOutput->setFontFamily("Monospace");
    m_edOutput->setMinimumWidth(280);

    m_btnGenerate = new QPushButton("Generate");
    m_btnClear    = new QPushButton("Clear");

    auto *vOutput = new QVBoxLayout;
    vOutput->addWidget(new QLabel("Register Table:"));
    vOutput->addWidget(m_edOutput);
    vOutput->addWidget(m_btnGenerate);
    vOutput->addWidget(m_btnClear);

    auto *root = new QHBoxLayout(central);
    root->addWidget(tabs, 1);
    root->addLayout(vOutput, 0);

    connect(m_btnGenerate, &QPushButton::clicked, this, &MainWindow::onGenerate);
    connect(m_btnClear,    &QPushButton::clicked, this, &MainWindow::onClear);

    connect(m_tabMain, &MainTab::chipVariantChanged, this, [this](ChipVariant v) {
        bool isDual = (v == ChipVariant::CS5518);
        m_tabLvds->setDualPortVisible(isDual);
    });

    // Wire resolution changes from LVDS tab to MIPI tab
    connect(m_tabLvds, &LvdsTab::timingChanged, this, [this]() {
        double pclk = m_tabLvds->timingParams().pixelClkMhz;
        int lanes = m_tabMipi->mipiConfig().laneCount;
        m_tabMipi->onPixelClockChanged(pclk, lanes);
    });

    statusBar()->showMessage("Ready");
}

BuildParams MainWindow::collectParams() const {
    BuildParams p;
    p.variant    = m_tabMain->chipVariant();
    p.cmdFmt     = m_tabMain->cmdFormat();
    p.bistEnable = m_tabMain->bistEnabled();
    p.bistPat    = m_tabMain->bistPattern();
    p.timing     = m_tabLvds->timingParams();
    p.port0      = m_tabLvds->port0Config();
    p.port1      = m_tabLvds->port1Config();
    p.dualPort   = m_tabLvds->dualPort();
    p.eightBit   = m_tabLvds->eightBit();
    p.lvdsFmt    = m_tabLvds->lvdsFormat();
    p.adv        = m_tabLvds->advancedCfg();
    p.mipi       = m_tabMipi->mipiConfig();
    p.clkSrc     = m_tabMipi->clkSource();
    p.extClkMhz  = m_tabMipi->extClkMhz();
    return p;
}

void MainWindow::onGenerate() {
    BuildParams p = collectParams();
    QString result = buildRegTable(p);
    if (result.isEmpty()) {
        QMessageBox::critical(this, "Error",
            "Failed to generate register table. Check MIPI lane routing configuration.");
        return;
    }
    m_edOutput->setPlainText(result);
    statusBar()->showMessage("Register table generated.");
}

void MainWindow::onClear() {
    m_edOutput->clear();
    statusBar()->showMessage("Cleared.");
}
