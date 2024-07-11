#ifndef STM_SPECTROMETR_H
#define STM_SPECTROMETR_H
#include <QSerialPort>
#include <QSerialPortInfo>

#include <QObject>

#define BLACK_SIZE 13
#define SPECTRUM_SIZE 3648

#pragma pack(push,1)
struct SpectrumData {
  unsigned short  dummy1[14];
  short int       black1[BLACK_SIZE];
  unsigned short  dummy2[3];
  short int       spectrum[SPECTRUM_SIZE];
  unsigned short  dummy[14];
};  //!< Spectrum data structure
Q_DECLARE_METATYPE(SpectrumData)
#pragma pack(pop)

class stm_spectrometr: public QObject
{
    Q_OBJECT
public:
    stm_spectrometr(quint16 exposition);
    ~stm_spectrometr();
    void setIs_ready_to_close(bool newIs_ready_to_close);
    bool is_ready_to_close() const;
    void change_exposition(const quint16 expo);
    bool getIs_spectrometr_connected() const;
    void setIs_cycle_update(bool is_cycle_update);
    quint16 exposition() const;

private:
    QSerialPortInfo m_qspi;
    QSerialPort m_spectrometr;
    QSerialPort m_gps;
    QString m_port_name;
    bool m_is_spectrometr_connected;
    bool m_is_ready_to_close;
    bool m_is_expo_changed;
    bool m_is_cycle_update;
    quint16 m_exposition;
    QByteArray barr;

public slots:
    void getData();

 private slots:
    void readStmData();
    void readGpsData();
    void setExpo(const quint16& expo);

signals:
    void data_is_ready(QVector<double>& channels,
                       QVector<double>& values,
                       double max);
    void ready_to_close();
    void request_spectr();
};

#endif // STM_SPECTROMETR_H
