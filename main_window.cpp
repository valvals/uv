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
    ui->comboBox_expositions->addItems({"1ms","10ms","100ms","200ms","300ms","500ms","700ms","900ms","1s","1.2s","1.4s","1.5s","1.7s","1.8s"});
    ui->widget_plot->addGraph();
    ui->widget_plot->xAxis->setRange(1,3648);
    connect(&m_stm,SIGNAL(data_is_ready(QVector<double>&,QVector<double>&, double)),SLOT(showPlot(QVector<double>&,QVector<double>&, double)));
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
   QTimer::singleShot(50,&m_stm,SLOT(getData()));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  m_stm.setIs_ready_to_close(true);
  event->ignore();
}

