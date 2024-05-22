#include "main_window.h"
#include "ui_main_window.h"
#include "QDebug"
#include <QtEndian>

constexpr uint16_t spectral_packet_size = 7384;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_is_spectrometr_connected = false;
    auto port_list = m_qspi.availablePorts();
    for(auto &&port:port_list){
        if(port.serialNumber()=="306538683339"){
        qDebug()<<port.serialNumber()<<port.portName();
        m_port_name = port.portName();
        m_spectrometr.setPortName(m_port_name);
        m_spectrometr.setBaudRate(921600);
        m_is_spectrometr_connected = m_spectrometr.open(QIODevice::ReadWrite);
        }
    }
    if(m_is_spectrometr_connected){
        m_spectrometr.write("e5000\n");
        m_spectrometr.waitForBytesWritten();
        m_spectrometr.waitForReadyRead();
        qDebug()<<"bytes size: "<<m_spectrometr.bytesAvailable();
        auto ba = m_spectrometr.readAll();
        qDebug()<<"response from stm: "<<qFromLittleEndian<qint16>(ba);
        /*m_spectrometr.write("r\n");
        m_spectrometr.waitForBytesWritten();
        m_spectrometr.waitForReadyRead();
        qDebug()<<"bytes size: "<<m_spectrometr.bytesAvailable();
        qDebug()<<"bytes written: "<<QString::fromStdString(m_spectrometr.readAll().toStdString());*/
        connect(&m_spectrometr, SIGNAL(readyRead()),SLOT(readStmData()));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    if(m_spectrometr.isOpen()){
        m_spectrometr.close();
    }
}

void MainWindow::readStmData()
{
   if(m_spectrometr.bytesAvailable() == 4){
   auto ba = m_spectrometr.readAll();
   qDebug()<<"response from stm: "<<qFromLittleEndian<qint16>(ba);
   return;
  }
   if(spectral_packet_size == m_spectrometr.bytesAvailable()){
       qDebug()<<m_spectrometr.readAll().size();
   }

}


void MainWindow::on_pushButton_testExpo_clicked()
{
    if(m_is_spectrometr_connected){
        //m_spectrometr.write("r\n");
        m_spectrometr.write("e5000\n");
    }
}


void MainWindow::on_pushButton_spectr_clicked()
{
    if(m_is_spectrometr_connected){
        m_spectrometr.write("r\n");
    }
}

