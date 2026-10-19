#pragma once
#include <QWidget>
#include "regbuilder.h"

QT_BEGIN_NAMESPACE
class QRadioButton;
class QCheckBox;
class QComboBox;
QT_END_NAMESPACE

// "MAIN" tab: chip variant, BIST test mode, and command format selection.
class MainTab : public QWidget {
    Q_OBJECT
public:
    explicit MainTab(QWidget *parent = nullptr);

    ChipVariant chipVariant() const;
    CmdFormat   cmdFormat()   const;
    bool        bistEnabled() const;
    BistPattern bistPattern() const;

private slots:
    void onChipChanged();

signals:
    void chipVariantChanged(ChipVariant v);

private:
    QRadioButton *m_rbCS5518;
    QRadioButton *m_rbCS5516;
    QRadioButton *m_rbMipiCmd;
    QRadioButton *m_rbIicCmd;
    QCheckBox    *m_cbBistEnable;
    QRadioButton *m_rbColorBar;
    QRadioButton *m_rbGrayBar;
    QRadioButton *m_rbMobileBar;
    QRadioButton *m_rb8bpc;
    QRadioButton *m_rb6bpc;
};
