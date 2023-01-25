#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <stdint.h>
#include "rttyboard.h"
#include "rtty.h"
#include "siglentspecan.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateRxData(uint8_t data);
    void updatePeakFreq(double freqMHz);
    void updateRadioState(RttyState state);

private slots:
    void on_refreshComportsBtn_clicked();

    void on_comportComboBox_activated(int index);

    void on_connectBtn_clicked();

    void on_startIdleBtn_clicked();

    void on_startRxBtn_clicked();

    void on_startTxBtn_clicked();

    void on_startVcoCalBtn_clicked();

    void on_connectSpecAnBtn_clicked();

    void on_setVcoVoltageBtn_clicked();

private:
    Ui::MainWindow *ui;
    QString m_comport;
    RttyBoard* rttyBoard;
    Rtty* rttyThread;
    SiglentSpecAn* specAn;
};
#endif // MAINWINDOW_H
