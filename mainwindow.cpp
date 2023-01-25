#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    rttyBoard = nullptr;
    rttyThread = nullptr;
    specAn = nullptr;
}

MainWindow::~MainWindow()
{
    specAn->terminate();
    rttyThread->terminate();
    delete specAn;
    delete rttyThread;
    delete rttyBoard;
    delete ui;
}

/****************/
/* PUBLIC SLOTS */
/****************/

void MainWindow::updateRxData(uint8_t data){
    ui->rxDataHexLabel->setText(QString("0x%1").arg((uint)data, 0, 16));
}


void MainWindow::updatePeakFreq(double freqMHz){
    ui->specAnPeakFreqLCDNum->display(freqMHz);
}

void MainWindow::updateRadioState(RttyState state){
    QString txt;
    txt += QString("MODE:        %1\r\n").arg(state.mode);
    txt += QString("FREQ:        %1 MHz\r\n").arg(state.freqMHz, 0, 'f', '4');
    txt += QString("MARK FREQ:   %1 kHz\r\n").arg(state.markFreq/1000.0, 0, 'f', '3');
    txt += QString("SPACE FREQ:  %1 kHz\r\n").arg(state.spaceFreq/1000.0, 0, 'f', '3');
    txt += QString("BAUDRATE:    %1 BAUD\r\n").arg(state.baudrate, 0, 'f', '3');
    txt += QString("TX DATA:     0x%1\r\n").arg(state.txData, 0, 16);
    txt += QString("RX DATA:     0x%1\r\n").arg(state.rxData, 0, 16);
    txt += QString("RX DATA RDY: %1\r\n").arg(state.rxDataRdy);
    txt += QString("RX TONE:     %1 kHz\r\n").arg(state.rxTone/1000.0, 0, 'f', '3');
    txt += QString("VCO DAC:     %1 V\r\n").arg(state.vcoDacVoltage, 0, 'f', '3');
    txt += QString("VCO FREQ CAL:%1 Hz\r\n").arg(state.vcoFreqCalValue, 0, 'g', '3');
    txt += QString("PA DAC:      %1 V").arg(state.paDacVoltage, 0, 'f', '3');
    ui->radioStateLabel->setText(txt);
}

void MainWindow::on_refreshComportsBtn_clicked()
{
    ui->comportComboBox->clear();

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for(const auto& port : ports){
        ui->comportComboBox->addItem(port.portName());
    }
}


void MainWindow::on_comportComboBox_activated(int index)
{
    m_comport = ui->comportComboBox->itemText(index);
    ui->statusbar->showMessage(m_comport);
}


void MainWindow::on_connectBtn_clicked()
{
    if(m_comport.length() > 3){
        rttyThread = new Rtty(m_comport);

        // CONNECT SIGNALS AND SLOTS
        connect(rttyThread, &Rtty::rxTone, ui->rxToneLcdNum, qOverload<double>(&QLCDNumber::display));    //
        connect(rttyThread, &Rtty::rxData, this, &MainWindow::updateRxData);
        connect(rttyThread, &Rtty::radioState, this, &MainWindow::updateRadioState);

        rttyThread->start();
    }
}


void MainWindow::on_startIdleBtn_clicked()
{
    if(rttyThread != nullptr){
        rttyThread->setMode(RttyBoard::Mode::IDLE);
        ui->currentModeLabel->setText("IDLE");
    }
}


void MainWindow::on_startRxBtn_clicked()
{
    if(rttyThread != nullptr){
        rttyThread->setMode(RttyBoard::Mode::RX);
        ui->currentModeLabel->setText("RX");
    }
}


void MainWindow::on_startTxBtn_clicked()
{
    if(rttyThread != nullptr){
        rttyThread->setMode(RttyBoard::Mode::TX);
        ui->currentModeLabel->setText("TX");
    }
}


void MainWindow::on_startVcoCalBtn_clicked()
{
    if(rttyThread != nullptr && specAn != nullptr){
        connect(specAn, &SiglentSpecAn::setVCOVoltage, rttyThread, &Rtty::setVCOVoltage);
        connect(specAn, &SiglentSpecAn::calPointComplete, rttyThread, &Rtty::setVCOCalFreq);

        rttyThread->setMode(RttyBoard::Mode::CALIBRATE_VCO);
        ui->currentModeLabel->setText("CALIBRATE VCO");
        specAn->startStopCalibration(true);
    }
}


void MainWindow::on_connectSpecAnBtn_clicked()
{
    QString ipAddr = ui->specAnComboBox->currentText();
    if(ipAddr.length() > 3){
        specAn = new SiglentSpecAn(ipAddr, this);
        ui->specAnIdnLabel->setText(specAn->getIdentity());
        specAn->setStartFreq(10.0e6);
        specAn->setStopFreq(40.0e6);
        specAn->setRBW(10000.0);
        specAn->setContPeak(true);
        connect(specAn, &SiglentSpecAn::peakFreqMHz, ui->specAnPeakFreqLCDNum, qOverload<double>(&QLCDNumber::display));
        specAn->start();
    }
}


void MainWindow::on_setVcoVoltageBtn_clicked()
{
    if(rttyThread != nullptr){
        bool success = false;
        double v = ui->vcoVoltageLineEdit->text().toDouble(&success);
        if(success && v >= 0.0 && v <= 3.3){
            rttyThread->setVCOVoltage(v);
        }

    }
}

