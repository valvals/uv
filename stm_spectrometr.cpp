#include "stm_spectrometr.h"
#include <QDebug>
#include <QtEndian>

constexpr uint16_t spectral_packet_size = 7384;

stm_spectrometr::stm_spectrometr(quint16 exposition)
{
    m_is_spectrometr_connected = false;
    m_is_ready_to_close = false;
    m_is_expo_changed = false;
    m_is_cycle_update = false;
    m_is_gps_connected = false;
    m_exposition = exposition;
    auto port_list = m_qspi.availablePorts();
    for(auto &&port:port_list){
        if(port.serialNumber()=="306538683339"){
            qDebug()<<port.serialNumber()<<port.portName();
            m_port_name = port.portName();
            m_spectrometr.setPortName(m_port_name);
            m_spectrometr.setBaudRate(115200);
            m_is_spectrometr_connected = m_spectrometr.open(QIODevice::ReadWrite);
        }else if(port.productIdentifier()==423){
            m_gps.setPortName(port.portName());
            m_is_gps_connected = m_gps.open(QIODevice::ReadWrite);
            connect(&m_gps, SIGNAL(readyRead()),SLOT(readGpsData()));
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

quint16 stm_spectrometr::exposition() const
{
    return m_exposition;
}

bool stm_spectrometr::getIs_gps_connected() const
{
    return m_is_gps_connected;
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
        for (int i = 0; i < BLACK_SIZE; ++i) {
            black_sum += spectrumData.black1[i];
            //qDebug()<<"black"<<i<<spectrumData.black1[i];
        }
        average_black = (double)black_sum / BLACK_SIZE;
        //qDebug()<<"av_black: "<<average_black;
        for (size_t i = 0; i < SPECTRUM_SIZE; ++i) {
            channels.push_back(i + 1);
            values.push_back(spectrumData.spectrum[i] - average_black);
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
            m_is_ready_to_close = true;
            return;
        }
        emit data_is_ready(channels, values, max);
    }

}

void stm_spectrometr::readGpsData()
{
      auto data = m_gps.readAll();
      QStringList allData;
      allData = QString(data).split("\n");
      for(int i=0;i<allData.count();i++){
              if(allData[i].contains("$GPGGA")){
                  QStringList data = allData[i].split(",");
                  //qDebug()<<data<<data.size();
                  if(data.size()<10) {
                      emit lat_long_alt_updated("NA","NA","NA");
                      return;
                  }
                  auto lat = parseNmea2grad(data[2],data[3]);
                  auto lnt = parseNmea2grad(data[4],data[5]);
                  auto alt = data[9];
                  emit lat_long_alt_updated(lat,lnt,alt);
              }
          }
      emit save_gps(data);

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

QString stm_spectrometr::parseNmea2grad(QString dm,QString nswe)
{
    QString result;
    int dec_degrees;
    float minutes;
    float my_deg;
    int plus_minus=1;

    if(nswe=="W"||nswe=="S"){plus_minus=-1;}else{plus_minus=1;}
    if(nswe=="N"||nswe=="S"){

        dec_degrees = dm.mid(0,2).toInt();
        minutes = dm.mid(2,dm.length()-1).toFloat();
        my_deg = dec_degrees+(minutes/60)*plus_minus;
        result = QString::number(static_cast<double>(my_deg),'g',8);
        //m_lattyNavi = static_cast<double>(my_deg);
        //m_latitude = result;

    }else{

        dec_degrees = dm.mid(0,3).toInt();
        minutes = dm.mid(3,dm.length()-1).toFloat();
        my_deg = dec_degrees+(minutes/60)*plus_minus;
        result = QString::number(static_cast<double>(my_deg),'g',8);
        //m_longyNavi = static_cast<double>(my_deg);
        //longitude = result;
    }
    return result;
}
