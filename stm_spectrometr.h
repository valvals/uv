#ifndef STM_SPECTROMETR_H
#define STM_SPECTROMETR_H
#include <QSerialPort>
#include <QSerialPortInfo>

#include <QObject>

#pragma pack(push,1)
struct SpectrumData {
  unsigned short  dummy1[14];
  short int       black1[13];
  unsigned short  dummy2[3];
  short int       spectrum[3648];
  unsigned short  dummy[14];
};  //!< Spectrum data structure
Q_DECLARE_METATYPE(SpectrumData)
#pragma pack(pop)

class stm_spectrometr: public QObject
{
    Q_OBJECT
public:
    stm_spectrometr();
    ~stm_spectrometr();



    void setIs_ready_to_close(bool newIs_ready_to_close);

    bool is_ready_to_close() const;

private:
    QSerialPortInfo m_qspi;
    QSerialPort m_spectrometr;
    QString m_port_name;
    bool m_is_spectrometr_connected;
    bool m_is_ready_to_close;
    QByteArray barr;
public slots:
    void getData();
 private slots:
    void readStmData();
    void setExpo(const quint16& expo);

signals:
    void data_is_ready(QVector<double>& channels,
                       QVector<double>& values,
                       double max);
};

#endif // STM_SPECTROMETR_H
