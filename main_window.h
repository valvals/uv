#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

;// hack unterminated warning pack
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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSerialPortInfo m_qspi;
    QSerialPort m_spectrometr;
    QString m_port_name;
    bool m_is_spectrometr_connected;
private slots:
    void readStmData();
    void on_pushButton_testExpo_clicked();
    void on_pushButton_spectr_clicked();
};
#endif // MAIN_WINDOW_H
