#include "rtty.h"

Rtty::Rtty(QString comport, QObject *parent)
    : QThread{parent}
{
    rttyBoard = new RttyBoard(comport, this);
    configMtx = new QMutex();

    for(uint8_t i = 0; i < NUM_FIELDS; i++){
        m_changeFieldFlags.insert(i, false);
    }

}

Rtty::~Rtty(){
    delete rttyBoard;
    delete configMtx;
}

void Rtty::run(){
    forever{
        /* MODE SWITCH */
        switch(m_mode){

        case RttyBoard::Mode::IDLE:{
            break;
        }case RttyBoard::Mode::RX:{

            m_rxTone = rttyBoard->getRxTone();
            emit rxTone(m_rxTone);
            QThread::usleep(100);

            bool rxDataRdy = rttyBoard->getRxDataReady();
            if(rxDataRdy){
                m_rxData = rttyBoard->getRxData();
                emit rxData(m_rxData);
            }
            QThread::usleep(100);
            break;
        }case RttyBoard::Mode::TX:{
            break;
        }case RttyBoard::Mode::CALIBRATE_VCO:{
            break;
        }

        };


        if(configMtx->tryLock()){
            // CHECK IF IT'S TIME TO CHANGE ANY FIELDS
            if(m_changeFieldFlags[FIELD_MODE]){
                rttyBoard->setMode(m_mode);
                m_changeFieldFlags[FIELD_MODE] = false;
                QThread::usleep(100);
            }
            if(m_changeFieldFlags[FIELD_FREQ_MHZ]){
                rttyBoard->setFrequency(m_freq);
                m_changeFieldFlags[FIELD_FREQ_MHZ] = false;
                QThread::usleep(100);
            }
            if(m_changeFieldFlags[FIELD_BAUD_RATE]){
                rttyBoard->setRttyBaudRate(m_baudRate);
                m_changeFieldFlags[FIELD_BAUD_RATE] = false;
                QThread::usleep(100);
            }
            if(m_changeFieldFlags[FIELD_VCO_DAC_VOLTAGE]){
                rttyBoard->setVcoVoltage(m_vcoVoltage);
                m_changeFieldFlags[FIELD_VCO_DAC_VOLTAGE] = false;
                QThread::usleep(100);
            }
            if(m_changeFieldFlags[FIELD_VCO_FREQ_CAL_VALUE]){
                rttyBoard->setVcoCalFreq(m_vcoCalFreq);
                m_changeFieldFlags[FIELD_VCO_FREQ_CAL_VALUE] = false;
                QThread::usleep(100);
            }
            configMtx->unlock();
        }

        RttyState state;
        rttyBoard->updateRttyState(&state);
        emit radioState(state);

    }
}


/* BEGIN SLOTS */

void Rtty::setMode(RttyBoard::Mode mode){
    if(mode != m_mode){
        if(configMtx->tryLock()){
            m_mode = mode;
            m_changeFieldFlags[FIELD_MODE] = true;
            configMtx->unlock();
        }
    }

}

void Rtty::setFrequency(double freq){
    if(freq != m_freq){
        if(configMtx->tryLock()){
            m_freq = freq;
            m_changeFieldFlags[FIELD_FREQ_MHZ] = true;
            configMtx->unlock();
        }
    }
}

void Rtty::setBaudRate(double baud){
    if(baud != m_baudRate){
        if(configMtx->tryLock()){
            m_baudRate = baud;
            m_changeFieldFlags[FIELD_BAUD_RATE] = true;
            configMtx->unlock();
        }
    }
}

void Rtty::setVCOVoltage(double voltage){
    if(voltage != m_vcoVoltage){
        if(configMtx->tryLock()){
            m_vcoVoltage = voltage;
            m_changeFieldFlags[FIELD_VCO_DAC_VOLTAGE] = true;
            configMtx->unlock();
        }
    }
}

void Rtty::setVCOCalFreq(double freq){
    if(freq != m_vcoCalFreq){
        if(configMtx->tryLock()){
            m_vcoCalFreq = freq;
            m_changeFieldFlags[FIELD_VCO_FREQ_CAL_VALUE] = true;
            configMtx->unlock();
        }
    }
}
