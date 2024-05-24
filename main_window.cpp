#include "main_window.h"
#include "ui_main_window.h"
#include "QDebug"
#include <cstring>
#include "unistd.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    jsn::getJsonArrayFromFile("expo_list.json",m_expositions);
    qDebug()<<"Expo size: "<<m_expositions.size();
    for(int i=0;i<m_expositions.size();++i){
        ui->comboBox_expositions->addItems(m_expositions[i].toObject().keys());
    }
    //ui->comboBox_expositions->addItems({"1ms","10ms","100ms","200ms","300ms","500ms","700ms","900ms","1s","1.2s","1.4s","1.5s","1.7s","1.8s"});
    ui->widget_plot->addGraph();
    ui->widget_plot->xAxis->setRange(1,3648);
    connect(&m_stm,SIGNAL(data_is_ready(QVector<double>&,QVector<double>&, double)),SLOT(showPlot(QVector<double>&,QVector<double>&, double)));
    connect(&m_stm,SIGNAL(ready_to_close()),this,SLOT(exit()));
    QTimer::singleShot(500,&m_stm,SLOT(getData()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_spectr_clicked()
{
    m_stm.getData();
}

void MainWindow::showPlot(QVector<double> &channels,
                          QVector<double> &values,
                          double max)
{
    //qDebug()<<channels.size()<<" --- "<<values.size();
    ui->widget_plot->graph(0)->setData(channels,values);
    ui->widget_plot->xAxis->setRange(1,channels.size());
    ui->widget_plot->yAxis->setRange(0,max);
    ui->widget_plot->replot();
    if(m_stm.is_ready_to_close()){
        qApp->exit(0);
    }
    QTimer::singleShot(50,&m_stm,SLOT(getData()));
}

void MainWindow::exit()
{
    qApp->exit(0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"close event....";
    if(m_stm.is_ready_to_close()){
        event->accept();
        return;
    }
    m_stm.setIs_ready_to_close(true);
    event->ignore();
}


void MainWindow::on_comboBox_expositions_currentIndexChanged(int index)
{
    static bool is_first = true;
    if(is_first){
        is_first = false;
        return;
    }
    auto value = m_expositions[index].toObject().value(ui->comboBox_expositions->currentText()).toDouble();
    m_stm.change_exposition(value);
}

