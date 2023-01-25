#ifndef SIGLENTSPECAN_H
#define SIGLENTSPECAN_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <visa.h>

#define MAX_CNT 1024
#define VCO_STEPS           512
#define VCO_VOLTAGE_STEP    (3.3/(VCO_STEPS - 1))

class SiglentSpecAn : public QThread
{
    Q_OBJECT
    void run() override;

    enum class State : int {
        IDLE,
        START_CALIBRATION,
        CALIBRATION,
        STOP_CALIBRATION
    };
public:
    explicit SiglentSpecAn(QString ipAddr, QObject *parent = nullptr);
    ~SiglentSpecAn();
    QString getIdentity();
    double getMarkerFreq();
    double getSweepTime();

public slots:
    void setRefLevel(int ref);
    void setStartFreq(double start);
    void setStopFreq(double start);
    void setCenterFreq(double center);
    void setFreqSpan(double span);
    void setRBW(double rbw);
    void setContPeak(bool on_off);
    void setMarkerAsCenter();
    void startStopCalibration(bool start_stop);

private:
    QMutex* m_configMtx;
    ViStatus m_status;
    ViSession m_defaultRM;
    ViSession m_instr;
    ViUInt32  m_retCount;
    unsigned char m_buffer[MAX_CNT];
    QString m_resrcStr;
    void sendCommand(QString cmd);
    QString query(QString cmd);
    QPair<double, QString> getFreqUnits(double freq);
    bool m_doCalibration;
    SiglentSpecAn::State m_calState;
    double m_vcoSetpt;

signals:
    void queryCmdResp(QString cmd_resp);
    void peakFreqMHz(double freq);
    void peakPower(double pwr);
    void setVCOVoltage(double voltage);
    void setRttyMode(uint8_t mode);
    void calPointComplete(double freq);
    void calibrationComplete();
};

#endif // SIGLENTSPECAN_H
