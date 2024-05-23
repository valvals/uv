#include "main_window.h"
#include "ui_main_window.h"
#include "QDebug"

#include <cstring>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->widget_plot->addGraph();
    ui->widget_plot->xAxis->setRange(1,3648);
    connect(&m_stm,SIGNAL(data_is_ready(QVector<double>&,QVector<double>&, double)),SLOT(showPlot(QVector<double>&,QVector<double>&, double)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_testExpo_clicked()
{

}


void MainWindow::on_pushButton_spectr_clicked()
{
    //ui->pushButton_spectr->setEnabled(false);
    m_stm.getData();
}

void MainWindow::showPlot(QVector<double> &channels,
                          QVector<double> &values,
                          double max)
{
   qDebug()<<channels.size()<<" --- "<<values.size();
   ui->widget_plot->graph(0)->setData(channels,values);
   ui->widget_plot->xAxis->setRange(1,channels.size());
   ui->widget_plot->yAxis->setRange(0,max);
   ui->widget_plot->replot();
   QTimer::singleShot(50,&m_stm,SLOT(getData()));
   //ui->pushButton_spectr->setEnabled(true);
}

