#include "stm_spectrometr.h"
#include <QDebug>
#include <QtEndian>

constexpr uint16_t spectral_packet_size = 7384;

stm_spectrometr::stm_spectrometr()
{
    m_is_spectrometr_connected = false;
    m_is_ready_to_close = false;
    m_is_expo_changed = false;
    m_is_cycle_update = false;
    m_exposition = 500;
    auto port_list = m_qspi.availablePorts();
    for(auto &&port:port_list){
        if(port.serialNumber()=="306538683339"){
            qDebug()<<port.serialNumber()<<port.portName();
            m_port_name = port.portName();
            m_spectrometr.setPortName(m_port_name);
            m_spectrometr.setBaudRate(115200);
            m_is_spectrometr_connected = m_spectrometr.open(QIODevice::ReadWrite);
        }
    }
    if(m_is_spectrometr_connected){
        qDebug()<<"bytes size: "<<m_spectrometr.bytesAvailable();
        m_spectrometr.waitForBytesWritten();
        m_spectrometr.write("e500\n");
        m_spectrometr.waitForBytesWritten();
        m_spectrometr.waitForReadyRead();
        QByteArray ba;
        ba = m_spectrometr.readAll();
        qDebug()<<"response from stm: "<<"ba size: "<<ba.size()<<qFromLittleEndian<qint32>(ba);
        m_is_cycle_update = true;
        connect(&m_spectrometr, SIGNAL(readyRead()),SLOT(readStmData()));
    }
}

stm_spectrometr::~stm_spectrometr()
{
    if(m_spectrometr.isOpen()){
        qDebug()<<"--------- BA ------------->"<<m_spectrometr.bytesAvailable();
        disconnect(&m_spectrometr,SIGNAL(readyRead()),this,SLOT(readStmData()));
        m_spectrometr.close();
    }
}

void stm_spectrometr::setIs_ready_to_close(bool newIs_ready_to_close)
{
    m_is_ready_to_close = newIs_ready_to_close;
}

bool stm_spectrometr::is_ready_to_close() const
{
    return m_is_ready_to_close;
}

void stm_spectrometr::change_exposition(const quint16 expo)
{
    m_is_expo_changed = true;
    m_exposition = expo;
}

bool stm_spectrometr::getIs_spectrometr_connected() const
{
    return m_is_spectrometr_connected;
}

void stm_spectrometr::setIs_cycle_update(bool is_cycle_update)
{
    m_is_cycle_update = is_cycle_update;
}

void stm_spectrometr::readStmData()
{
    barr.append(m_spectrometr.readAll());
    if(barr.size() == 4){
        qDebug()<<"response from stm: "<<qFromLittleEndian<qint16>(barr);
        barr.clear();
        if(m_is_ready_to_close){
            emit ready_to_close();
        }
        if(m_is_expo_changed){
            m_is_expo_changed = false;
            getData();
        }
        return;
    }

    if(spectral_packet_size == barr.size()){

        SpectrumData spectrumData;
        memcpy(&spectrumData,barr, sizeof(spectrumData));
        QVector<double> values;
        QVector<double> channels;
        double max = 0;
        double average_black = 0.0;
        int black_sum = 0;
        int black_array_size = sizeof(spectrumData.black1);
        for (int i = 0; i < black_array_size; ++i) {
            black_sum += spectrumData.black1[i];
            //qDebug()<<"black"<<i<<spectrumData.black1[i];
        }
        average_black = (double)black_sum / (double)black_array_size;
        //qDebug()<<"av_black: "<<average_black;
        for (size_t i = 0; i < 3648; ++i) {
            channels.push_back(i + 1);
            values.push_back(spectrumData.spectrum[i]);// - average_black);
            if (max < spectrumData.spectrum[i])
                max = spectrumData.spectrum[i];
        };
        barr.clear();
        if(m_is_ready_to_close){
            emit ready_to_close();
            return;
        }
        if(m_is_expo_changed){
            auto command = QString("e%1\n").arg(m_exposition).toLatin1();
            qDebug()<<command;
            m_spectrometr.write(command);
            m_spectrometr.waitForBytesWritten();
            return;
        }
        if(m_is_cycle_update == false){
            return;
        }
        emit data_is_ready(channels, values, max);
    }

}

void stm_spectrometr::getData()
{
    if(m_is_spectrometr_connected){
        m_spectrometr.write("r\n");
    }
}

void stm_spectrometr::setExpo(const quint16& expo)
{
    if(m_is_spectrometr_connected){
        m_spectrometr.write(QString("e%1\n").arg(expo).toLatin1());
    }
}
