#include "rttyboard.h"
#include <cstring>

RttyBoard::RttyBoard(QString& comport, QObject *parent)
    : QObject{parent}
{
    m_ser = new QSerialPort(comport);
    m_ser->setBaudRate(QSerialPort::Baud115200);
    if(!m_ser->isOpen()){
        m_ser->open(QIODeviceBase::ReadWrite);
    }
}

RttyBoard::~RttyBoard(){
    m_ser->close();
    delete m_ser;
}

void RttyBoard::updateRttyState(RttyState* state){
    QList<uint8_t> all_fields;
    for(uint8_t i = 0; i < NUM_FIELDS; i++){
        all_fields.append(i);
    }

    auto field_data = readFields(all_fields);
    uint32_t tmp;
    for(auto pair : field_data){
        tmp = pair.second;
        switch(pair.first){
        case FIELD_MODE:
            state->mode = (int)tmp;
            break;
        case FIELD_FREQ_MHZ:
            memcpy(&(state->freqMHz), &tmp, 4);
            break;
        case FIELD_MARK_FREQ:
            memcpy(&(state->markFreq), &tmp, 4);
            break;
        case FIELD_SPACE_FREQ:
            memcpy(&(state->spaceFreq), &tmp, 4);
            break;
        case FIELD_BAUD_RATE:
            memcpy(&(state->baudrate), &tmp, 4);
            break;
        case FIELD_TX_DATA:
            memcpy(&(state->txData), &tmp, 4);
            break;
        case FIELD_RX_DATA:
            memcpy(&(state->rxData), &tmp, 4);
            break;
        case FIELD_RX_DATA_RDY:
            state->rxDataRdy = (bool)tmp;
            break;
        case FIELD_RX_TONE:
            memcpy(&(state->rxTone), &tmp, 4);
            break;
        case FIELD_VCO_DAC_VOLTAGE:
            memcpy(&(state->vcoDacVoltage), &tmp, 4);
            break;
        case FIELD_VCO_FREQ_CAL_VALUE:
            memcpy(&(state->vcoFreqCalValue), &tmp, 4);
            break;
        case FIELD_PA_DAC_VOLTAGE:
            memcpy(&(state->paDacVoltage), &tmp, 4);
            break;

        }
    }

}

/*************************/
/* BEGIN PRIVATE METHODS */
/*************************/

/**
 * @brief RttyBoard::readFields query each field listed in fields argument
 * @param fields list of field indexes to query
 * @return list of index:value pairs read from the board
 */
QList<QPair<uint8_t, uint32_t>> RttyBoard::readFields(QList<uint8_t>& fields){
    QList<QPair<uint8_t, uint32_t>> retval;

    QByteArray cmd_buf(2 + fields.length(), 0x00);

    cmd_buf[0] = CMD_READ_FIELDS;
    cmd_buf[1] = fields.length();
    int i = 2;
    for(auto field : fields){
        cmd_buf[i] = field;
        i++;
    }

    m_ser->write(cmd_buf);

    QByteArray rpy_buf(2 + 5*fields.length(), 0x00);
    char* rpy = rpy_buf.data();
    m_ser->read(rpy, 2);
    int len = rpy[1];
    m_ser->read(rpy + 2, len);

    i = 2;
    while(i < len + 2){
        uint32_t data;
        memcpy(&data, rpy + i + 1, 4);
        QPair<uint8_t, uint32_t> pair(rpy[i], data);
        retval.append(pair);
    }

    return retval;
}

/**
 * @brief RttyBoard::setFields set fields to values specified by the fields arg
 * @param fields a list of index:value pairs
 */
void RttyBoard::setFields(QList<QPair<uint8_t, uint32_t>>& fields){
    QByteArray cmd_buf(2 + 5*fields.length(), 0x00);
    cmd_buf[0] = CMD_SET_FIELDS;
    cmd_buf[1] = 5*fields.length();
    char* cmd = cmd_buf.data();

    int i = 2;
    for(auto pair: fields){
        cmd[i] = pair.first;
        i++;
        memcpy(cmd + i, &pair.second, 4);
        i += 4;
    }

    m_ser->write(cmd_buf);

}


void RttyBoard::setField(uint8_t field, uint32_t value){
    QList<QPair<uint8_t, uint32_t>> list;
    list.append(QPair<uint8_t, uint32_t>(field, value));
    setFields(list);
}


uint32_t RttyBoard::readField(uint8_t field){
    QList<uint8_t> fields;
    fields.append(field);
    auto rpy = readFields(fields);
    return rpy[0].second;
}

float RttyBoard::readFieldFloat(uint8_t field){
    uint32_t val = readField(field);
    float retval;
    memcpy(&retval, &val, 4);
    return retval;
}


void RttyBoard::setFieldFloat(uint8_t field, float value){
    uint32_t n_value = 0;
    memcpy(&n_value, &value, 4);
    setField(field, n_value);
}

/**********************/
/* BEGIN PUBLIC SLOTS */
/**********************/
RttyBoard::Mode RttyBoard::getMode(){
    return (RttyBoard::Mode)readField((uint8_t)FIELD_MODE);
}
void RttyBoard::setMode(RttyBoard::Mode mode){
    uint32_t n_mode = (uint32_t)mode;
    setField((uint8_t)FIELD_MODE, n_mode);
}

float RttyBoard::getFrequency(){
    float MHz = readFieldFloat((uint8_t)FIELD_FREQ_MHZ);
    return MHz*1.0e6;
}
void RttyBoard::setFrequency(double freq){
    float MHz = (float)freq/1.0e6;
    setFieldFloat((uint8_t)FIELD_FREQ_MHZ, MHz);
}

float RttyBoard::getRttyBaudRate(){
    return readFieldFloat((uint8_t)FIELD_BAUD_RATE);
}
void RttyBoard::setRttyBaudRate(double baud){
    setFieldFloat((uint8_t)FIELD_BAUD_RATE, (float)baud);
}

void RttyBoard::setVcoVoltage(double voltage){
    setFieldFloat((uint8_t)FIELD_VCO_DAC_VOLTAGE, (float)voltage);
}

void RttyBoard::setVcoCalFreq(double freq){
    setFieldFloat((uint8_t)FIELD_VCO_FREQ_CAL_VALUE, (float)freq);
}

float RttyBoard::getRxTone(){
    return readFieldFloat((uint8_t)FIELD_RX_TONE);
}

bool RttyBoard::getRxDataReady(){
    return (bool)readField((uint8_t)FIELD_RX_DATA_RDY);
}

uint8_t RttyBoard::getRxData(){
    return (uint8_t)readField((uint8_t)FIELD_RX_DATA);
}
