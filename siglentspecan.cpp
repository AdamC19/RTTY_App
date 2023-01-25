#include "siglentspecan.h"
#include <QDebug>
#include <string>

/********************/
/* PUBLIC FUNCTIONS */
/********************/
std::string qStringToBasic(QString qstr){
    std::string temp;
    for(int i = 0; i < qstr.size(); i++){
        temp.append(1, qstr[i].toLatin1());
    }
    return temp;
}

/******************/
/* PUBLIC METHODS */
/******************/

SiglentSpecAn::SiglentSpecAn(QString ipAddr, QObject *parent)
    : QThread{parent}
{
    m_status = viOpenDefaultRM(&m_defaultRM);
    if(m_status < VI_SUCCESS){
        qDebug() << "Error opening VISA resource manager...";
        // error
    }
    m_resrcStr = QString("TCPIP0::%1::inst0::INSTR").arg(ipAddr);
    std::string temp;
    for(int i = 0; i < m_resrcStr.size(); i++){
        temp.append(1, m_resrcStr[i].toLatin1());
    }
    qDebug() << "Opening Siglent spectrum analyzer with resource string " << temp.c_str() << "...";
    m_status = viOpen(m_defaultRM, temp.c_str(), VI_NULL, VI_NULL, &m_instr);
    if(m_status < VI_SUCCESS){
        qDebug() << "Error opening Siglent spectrum analyzer with resource string " << temp.c_str() << "...";
        qDebug() << QString("0x%1").arg(m_status, 0, 16);
        // error
    }
    m_status = viSetAttribute(m_instr, VI_ATTR_TMO_VALUE, 2000);

    m_calState = SiglentSpecAn::State::IDLE;

    m_configMtx = new QMutex();
}

SiglentSpecAn::~SiglentSpecAn(){
    m_status = viClose(m_instr);
    m_status = viClose(m_defaultRM);
}

QString SiglentSpecAn::getIdentity(){
    return query("*IDN?\n");
}

double SiglentSpecAn::getMarkerFreq(){
    QString rpy = query(":CALC:MARKer1:X?\n");
    return (double)rpy.toFloat();
}

double SiglentSpecAn::getSweepTime(){
    QString rpy = query(":SENSe:SWEep:TIME?\n");
    return (double)rpy.toFloat();
}

/*******************/
/* PRIVATE METHODS */
/*******************/
void SiglentSpecAn::run(){
    forever{
        switch(m_calState){
        case SiglentSpecAn::State::IDLE:{
            // spit out data just for fun
            if(m_configMtx->tryLock()){
                emit peakFreqMHz(getMarkerFreq()/1.0e6);
                m_configMtx->unlock();
            }
            break;
        }case SiglentSpecAn::State::START_CALIBRATION:{
            setRBW(100000.0);
            setStartFreq(10.0e6);
            setStopFreq(40.0e6);
            setRefLevel(10); // 10 dBm
            setContPeak(true);
            QThread::usleep(100000);
            setMarkerAsCenter(); // peak becomes center of the spectrum
            setFreqSpan(500000.0);
            setRBW(300.0);

            m_calState = SiglentSpecAn::State::CALIBRATION;
            m_vcoSetpt = 0.0;
            break;
        }case SiglentSpecAn::State::CALIBRATION:{
            emit setVCOVoltage(m_vcoSetpt);
            QThread::usleep(250000); // allow things to settle
            setMarkerAsCenter(); // center the peak on the screen
            QThread::usleep(50000);

            double avgFreq = 0.0;
            int sweepTime = (int)(getSweepTime()*1.0e6);
            for(int i = 0; i < 10; i++){
                avgFreq += getMarkerFreq();
                QThread::usleep(sweepTime + 1000);
            }
            emit calPointComplete(avgFreq/10.0);

            m_vcoSetpt += VCO_VOLTAGE_STEP;
            if(m_vcoSetpt > 3.3){
                m_calState = SiglentSpecAn::State::STOP_CALIBRATION;
            }
            break;
        }case SiglentSpecAn::State::STOP_CALIBRATION:{
            emit calibrationComplete();
            m_calState = SiglentSpecAn::State::IDLE;
            break;
        }
        };


        QThread::usleep(10000);
    }
}
QString SiglentSpecAn::query(QString cmd){
    sendCommand(cmd);
    m_status = viRead(m_instr, m_buffer, MAX_CNT, &m_retCount);
    QString retval = QString((const char*)m_buffer);
    retval.truncate(m_retCount);
    QString cmd_resp = QString("%1 -> %2").arg(cmd.trimmed(), retval.trimmed());
//    qDebug() << cmd_resp;
    emit queryCmdResp(cmd_resp);
    return retval.trimmed();
}

void SiglentSpecAn::sendCommand(QString cmd){
    if(!cmd.endsWith('\n')){
        cmd += '\n';
    }
    std::string temp = qStringToBasic(cmd);
    m_status = viWrite(m_instr, (const unsigned char*)temp.c_str(), cmd.length(), &m_retCount);
}

QPair<double, QString> SiglentSpecAn::getFreqUnits(double freq){
    QPair<double, QString> retval;
    if(freq < 1.0e6){
        retval.second = "kHz";
        retval.first = freq / 1000.0;
    }else if(freq < 1.0e9){
        retval.second = "MHz";
        retval.first = freq / 1.0e6;
    }else{
        retval.second = "GHz";
        retval.first = freq / 1.0e9;
    }
    return retval;
}


/****************/
/* PUBLIC SLOTS */
/****************/


void SiglentSpecAn::setRefLevel(int ref){
    if(m_configMtx->tryLock()){
        QString cmd = QString(":DISPlay:WINDow:TRACe:Y:RLEVel %1 DBM\n").arg(ref);
        sendCommand(cmd);
        m_configMtx->unlock();
    }
}

void SiglentSpecAn::setStartFreq(double start){
    if(m_configMtx->tryLock()){
        auto freqUnits = getFreqUnits(start);
        QString cmd = QString(":FREQuency:STARt %1 %2\n").arg(freqUnits.first, 0, 'f', 6).arg(freqUnits.second);
        sendCommand(cmd);
        m_configMtx->unlock();
    }
}
void SiglentSpecAn::setStopFreq(double stop){
    if(m_configMtx->tryLock()){
        auto freqUnits = getFreqUnits(stop);
        QString cmd = QString(":FREQuency:STOP %1 %2\n").arg(freqUnits.first, 0, 'f', 6).arg(freqUnits.second);
        sendCommand(cmd);
        m_configMtx->unlock();
    }
}
void SiglentSpecAn::setCenterFreq(double center){
    if(m_configMtx->tryLock()){
        auto centerUnits = getFreqUnits(center);
        QString cmd = QString(":FREQuency:CENTer %1 %2\n").arg(centerUnits.first, 0, 'f', 6).arg(centerUnits.second);
        sendCommand(cmd);
        m_configMtx->unlock();
    }
}
void SiglentSpecAn::setFreqSpan(double span){
    if(m_configMtx->tryLock()){
        auto spanUnits = getFreqUnits(span);
        QString cmd = QString(":FREQuency:SPAN %1 %2\n").arg(spanUnits.first, 0, 'f', 6).arg(spanUnits.second);
        sendCommand(cmd);
        m_configMtx->unlock();
    }
}
void SiglentSpecAn::setRBW(double rbw){
    if(m_configMtx->tryLock()){
        auto freqUnits = getFreqUnits(rbw);
        QString cmd = QString(":BWIDth:RESolution %1 %2\n").arg(freqUnits.first, 0, 'f', 6).arg(freqUnits.second);
        sendCommand(cmd);
        m_configMtx->unlock();
    }
}

void SiglentSpecAn::setContPeak(bool on_off){
    if(m_configMtx->tryLock()){
        if(on_off){
            QString cmd = QString(":CALCulate:MARKer1:CPEak:STATe ON\n");
            sendCommand(cmd);
        }else{
            QString cmd = QString(":CALCulate:MARKer1:CPEak:STATe OFF\n");
            sendCommand(cmd);
        }
        m_configMtx->unlock();
    }
}

void SiglentSpecAn::setMarkerAsCenter(){
    if(m_configMtx->tryLock()){
        QString cmd = QString(":CALCulate:MARKer1:CENTer\n");
        sendCommand(cmd);
        m_configMtx->unlock();
    }
}

void SiglentSpecAn::startStopCalibration(bool start_stop){

    if(!m_doCalibration && start_stop){
        m_calState = SiglentSpecAn::State::START_CALIBRATION;
    }else if(m_doCalibration && !start_stop){
        m_calState = SiglentSpecAn::State::STOP_CALIBRATION;
    }

    m_doCalibration = start_stop;
}
