#include "mipitab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>

static const QStringList kLaneItems = {"1","2","3","4"};
static const QStringList kLaneMapItems = {"Lane0","Lane1","Lane2","Lane3"};

MipiTab::MipiTab(QWidget *parent) : QWidget(parent) {
    // --- MIPI config ---
    auto *gbMipi = new QGroupBox("MIPI Configuration");
    m_cmbLaneCount = new QComboBox;
    m_cmbLaneCount->addItems(kLaneItems);
    m_cmbLaneCount->setCurrentIndex(3); // 4 lanes default

    m_edMipiClk = new QLineEdit("445.50");
    m_edMipiClk->setFixedWidth(90);
    m_edMipiClk->setAlignment(Qt::AlignRight);

    auto *frmMipi = new QFormLayout;
    auto *hLane = new QHBoxLayout;
    hLane->addWidget(m_cmbLaneCount); hLane->addStretch();
    frmMipi->addRow("Lane count:", hLane);
    auto *hMipiClk = new QHBoxLayout;
    hMipiClk->addWidget(m_edMipiClk); hMipiClk->addWidget(new QLabel("MHz"));
    frmMipi->addRow("MIPI clock:", hMipiClk);
    gbMipi->setLayout(frmMipi);

    // --- Clock source ---
    auto *gbClk = new QGroupBox("Reference Clock Source");
    m_rbMipiClk = new QRadioButton("Derived from MIPI clock");
    m_rbExtClk  = new QRadioButton("External clock:");
    m_rbInt27M  = new QRadioButton("Internal 27 MHz");
    m_rbMipiClk->setChecked(true);
    m_edExtClk = new QLineEdit("26");
    m_edExtClk->setFixedWidth(60);
    m_edExtClk->setAlignment(Qt::AlignRight);
    m_edExtClk->setEnabled(false);

    auto *hExt = new QHBoxLayout;
    hExt->addWidget(m_rbExtClk);
    hExt->addWidget(m_edExtClk);
    hExt->addWidget(new QLabel("MHz (20–50)"));
    hExt->addStretch();

    auto *vClk = new QVBoxLayout;
    vClk->addWidget(m_rbMipiClk);
    vClk->addLayout(hExt);
    vClk->addWidget(m_rbInt27M);
    gbClk->setLayout(vClk);

    // --- P/N swap ---
    auto *gbPnSwap = new QGroupBox("MIPI P/N Swap");
    auto *hPn = new QHBoxLayout;
    static const char *kLaneNames[] = {"Lane0","Lane1","Lane2","Lane3"};
    for (int i = 0; i < 4; ++i) {
        m_cbLaneSwap[i] = new QCheckBox(kLaneNames[i]);
        hPn->addWidget(m_cbLaneSwap[i]);
    }
    gbPnSwap->setLayout(hPn);

    // --- Lane routing ---
    auto *gbLaneMap = new QGroupBox("MIPI Lane Routing (chip lane → panel lane)");
    auto *frmMap = new QFormLayout;
    for (int i = 0; i < 4; ++i) {
        m_cmbLaneMap[i] = new QComboBox;
        m_cmbLaneMap[i]->addItems(kLaneMapItems);
        m_cmbLaneMap[i]->setCurrentIndex(i);
        frmMap->addRow(QString("Chip Lane %1:").arg(i), m_cmbLaneMap[i]);
    }
    gbLaneMap->setLayout(frmMap);

    auto *root = new QVBoxLayout(this);
    root->addWidget(gbMipi);
    root->addWidget(gbClk);
    root->addWidget(gbPnSwap);
    root->addWidget(gbLaneMap);
    root->addStretch();

    connect(m_cmbLaneCount, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MipiTab::onLaneCountChanged);
    connect(m_rbMipiClk, &QRadioButton::toggled, this, &MipiTab::onClkSourceChanged);
    connect(m_rbExtClk,  &QRadioButton::toggled, this, &MipiTab::onClkSourceChanged);
    connect(m_rbInt27M,  &QRadioButton::toggled, this, &MipiTab::onClkSourceChanged);
}

void MipiTab::onLaneCountChanged() {
    recalcMipiClock();
}

void MipiTab::onClkSourceChanged() {
    m_edExtClk->setEnabled(m_rbExtClk->isChecked());
}

void MipiTab::onPixelClockChanged(double pixClkMhz, int laneCount) {
    m_pixelClkMhz = pixClkMhz;
    recalcMipiClock();
}

void MipiTab::recalcMipiClock() {
    int lanes = m_cmbLaneCount->currentIndex() + 1;
    double mipiClk = m_pixelClkMhz * 12.0 / lanes;
    m_edMipiClk->setText(QString::number(mipiClk, 'f', 2));
}

MipiCfg MipiTab::mipiConfig() const {
    MipiCfg cfg;
    cfg.laneCount  = m_cmbLaneCount->currentIndex() + 1;
    cfg.mipiClkMhz = m_edMipiClk->text().toDouble();
    for (int i = 0; i < 4; ++i) {
        cfg.laneSwap[i] = m_cbLaneSwap[i]->isChecked();
        cfg.laneMap[i]  = m_cmbLaneMap[i]->currentIndex();
    }
    return cfg;
}

ClkSource MipiTab::clkSource() const {
    if (m_rbExtClk->isChecked())  return ClkSource::External;
    if (m_rbInt27M->isChecked())  return ClkSource::Internal27M;
    return ClkSource::MipiDerived;
}

double MipiTab::extClkMhz() const {
    return m_edExtClk->text().toDouble();
}
