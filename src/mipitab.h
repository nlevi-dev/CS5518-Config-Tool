#pragma once
#include <QWidget>
#include "regbuilder.h"

QT_BEGIN_NAMESPACE
class QRadioButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
QT_END_NAMESPACE

// "MIPI" tab: lane count, clock source, MIPI clock, P/N swap, lane routing.
class MipiTab : public QWidget {
    Q_OBJECT
public:
    explicit MipiTab(QWidget *parent = nullptr);

    MipiCfg   mipiConfig()  const;
    ClkSource clkSource()   const;
    double    extClkMhz()   const;

public slots:
    void onPixelClockChanged(double pixClkMhz, int laneCount);

private slots:
    void onLaneCountChanged();
    void onClkSourceChanged();

private:
    void recalcMipiClock();

    QComboBox    *m_cmbLaneCount;
    QRadioButton *m_rbMipiClk;
    QRadioButton *m_rbExtClk;
    QRadioButton *m_rbInt27M;
    QLineEdit    *m_edExtClk;
    QLineEdit    *m_edMipiClk;

    // P/N swap per lane
    QCheckBox    *m_cbLaneSwap[4];
    // Lane routing
    QComboBox    *m_cmbLaneMap[4];

    double m_pixelClkMhz = 148.5;
};
