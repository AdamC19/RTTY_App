#ifndef RTTY_H
#define RTTY_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QMap>

#include "rttyboard.h"

class Rtty : public QThread
{
    Q_OBJECT
    void run() override;
private:
    RttyBoard* rttyBoard;
    QMutex* configMtx;
    float m_rxTone;
    QMap<uint8_t, bool> m_changeFieldFlags;
    double m_freq;
    RttyBoard::Mode m_mode;
    double m_baudRate;
    uint8_t m_rxData;
    double m_vcoVoltage;
    double m_vcoCalFreq;

public:
    Rtty(QString comport, QObject *parent = nullptr);
    ~Rtty();

public slots:
    void setMode(RttyBoard::Mode mode);
    void setFrequency(double freq);
    void setBaudRate(double baud);
    void setVCOVoltage(double voltage);
    void setVCOCalFreq(double freq);

signals:
    void rxTone(float tone);
    void rxData(uint8_t data);
    void radioState(RttyState state);
};

#endif // RTTY_H
