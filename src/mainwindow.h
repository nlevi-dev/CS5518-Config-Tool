#pragma once
#include <QMainWindow>
#include "regbuilder.h"

QT_BEGIN_NAMESPACE
class QTabWidget;
class QTextEdit;
class QPushButton;
QT_END_NAMESPACE

class MainTab;
class LvdsTab;
class MipiTab;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onGenerate();
    void onClear();

private:
    BuildParams collectParams() const;

    MainTab  *m_tabMain;
    LvdsTab  *m_tabLvds;
    MipiTab  *m_tabMipi;
    QTextEdit    *m_edOutput;
    QPushButton  *m_btnGenerate;
    QPushButton  *m_btnClear;
};
