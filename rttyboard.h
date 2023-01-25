#ifndef RTTYBOARD_H
#define RTTYBOARD_H

#include <QObject>
#include <QSerialPort>
#include <inttypes.h>

enum commands_enum {
    CMD_NONE,
    CMD_READ_FIELDS,
    CMD_SET_FIELDS,
    CMD_TEST_COMMS
};

enum fields_enum {
    FIELD_MODE,
    FIELD_FREQ_MHZ,
    FIELD_MARK_FREQ,
    FIELD_SPACE_FREQ,
    FIELD_BAUD_RATE,
    FIELD_TX_DATA,
    FIELD_RX_DATA_RDY,
    FIELD_RX_DATA,
    FIELD_RX_TONE,
    FIELD_VCO_DAC_VOLTAGE,
    FIELD_VCO_FREQ_CAL_VALUE,
    FIELD_PA_DAC_VOLTAGE,
    NUM_FIELDS
};


typedef struct rtty_state_struct {
    int mode;
    float freqMHz;
    float markFreq;
    float spaceFreq;
    float baudrate;
    int txData;
    int rxData;
    bool rxDataRdy;
    float rxTone;
    float vcoDacVoltage;
    float vcoFreqCalValue;
    float paDacVoltage;
}RttyState;


class RttyBoard : public QObject
{
    Q_OBJECT

public:
    enum class Mode : int {
        IDLE = 0,
        RX = 1,
        TX = 2,
        CALIBRATE_VCO = 3
    };

    RttyBoard(QString& comport, QObject *parent = nullptr);
    ~RttyBoard();
    float getFrequency();
    float getRttyBaudRate();
    float getRxTone();
    bool getRxDataReady();
    uint8_t getRxData();
    void updateRttyState(RttyState* state);

private:
    QSerialPort* m_ser;
    QList<QPair<uint8_t, uint32_t>> readFields(QList<uint8_t>& fields);
    void setFields(QList<QPair<uint8_t, uint32_t>>& fields);
    void setField(uint8_t field, uint32_t value);
    uint32_t readField(uint8_t field);
    float readFieldFloat(uint8_t field);
    void setFieldFloat(uint8_t field, float value);

public slots:
    RttyBoard::Mode getMode();
    void setMode(RttyBoard::Mode mode);
    void setFrequency(double freq);
    void setRttyBaudRate(double baud);
    void setVcoVoltage(double voltage);
    void setVcoCalFreq(double freq);

signals:

};

#endif // RTTYBOARD_H
