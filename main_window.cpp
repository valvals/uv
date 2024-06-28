#include "main_window.h"
#include "ui_main_window.h"
#include "QDebug"
#include <cstring>
#include <QProcess>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(480,800);
    this->layout()->setSizeConstraint( QLayout::SetFixedSize );
    m_is_camera_picture_ready = false;
    //m_media_player = new QMediaPlayer;
    //m_media_player->setMedia(QUrl("rtsp://127.0.0.1:8554/stream1"));
    //m_video_widget = new QVideoWidget;
    //m_media_player->setVideoOutput(m_video_widget);
    //m_video_widget->show();
    //m_media_player->play();
    connect(&m_cam_process,SIGNAL(finished(int)),SLOT(update_camera_image(int)));
    connect(&m_time_timer,&QTimer::timeout,[this](){
        ui->label_time->setText(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd hh:mm:ss"));
    });
    m_time_timer.start(1000);
    QStringList modes = {"Авто","Небо","Газ"};
    ui->comboBox_mode->addItems(modes);
    jsn::getJsonArrayFromFile("expo_list.json",m_expositions);
    jsn::getJsonArrayFromFile("objects.json",m_objects);
    for(int i=0;i<m_objects.size();++i){
        ui->comboBox_objects->addItem(m_objects[i].toObject()["name"].toString());
    }
    m_stm = new stm_spectrometr(500);

    qDebug()<<"Expo size: "<<m_expositions.size();
    bool is500 = false;
    for(int i=0;i<m_expositions.size();++i){
        if(m_expositions[i].toObject().keys().size()>0){
        QString key = m_expositions[i].toObject().keys()[0];
        ui->comboBox_expositions->addItem(key);
        if(key == "500 ms"){is500 = true;}
        }
    }
    if(is500 == false){
        QJsonObject obj;
        obj["500 ms"] = 500;
        m_expositions.append(obj);
        ui->comboBox_expositions->addItem("500 ms");
    }
    ui->comboBox_expositions->setCurrentText("500 ms");
    ui->widget_plot->addGraph();
    ui->widget_plot->xAxis->setRange(1,3648);
    connect(m_stm,SIGNAL(data_is_ready(QVector<double>&,QVector<double>&, double)),SLOT(showPlot(QVector<double>&,QVector<double>&, double)));
    connect(m_stm,SIGNAL(ready_to_close()),this,SLOT(exit()));
    if(m_stm->getIs_spectrometr_connected()){
        ui->pushButton_spectr->setText("stop");
        QTimer::singleShot(500, m_stm, SLOT(getData()));
    }else{
        ui->pushButton_spectr->setText("disconnected");
    }
    start_capture_process();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::start_capture_process()
{
    m_is_camera_picture_ready = false;
    m_cam_process.start("libcamera-still --immediate -n  -v 0 --shutter 1000 awbgains=0.3,0.5  -o rp_image.jpg");
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
    if(m_stm->is_ready_to_close()){
        qApp->exit(0);
    }
    QTimer::singleShot(50,m_stm,SLOT(getData()));
    if(m_is_camera_picture_ready){
        QPixmap pm = QPixmap("rp_image.jpg");
        QMatrix rm;
        rm.rotate(90);
        pm = pm.transformed(rm);
        ui->label_camera->setPixmap(pm);

        start_capture_process();
    }
}

void MainWindow::update_camera_image(int)
{
    //qDebug()<<"------------------------------";
m_is_camera_picture_ready = true;
}

void MainWindow::exit()
{
    qApp->exit(0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"close event....";
    if(m_stm->getIs_spectrometr_connected()==false){
        event->accept();
        return;
    }
    if(m_stm->is_ready_to_close()){
        event->accept();
        return;
    }
    m_stm->setIs_ready_to_close(true);
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
    qDebug()<<"exposition value: "<<value;
    m_stm->change_exposition(value);
}


void MainWindow::on_pushButton_spectr_toggled(bool checked)
{
    if(m_stm->getIs_spectrometr_connected()==false){
        return;
    }
    if(checked){
        ui->pushButton_spectr->setText("stop");
        m_stm->setIs_ready_to_close(false);
        m_stm->setIs_cycle_update(true);
        m_stm->getData();
    }else{
        ui->pushButton_spectr->setText("start");
        m_stm->setIs_cycle_update(false);
    }
}

void MainWindow::on_pushButton_record_toggled(bool checked)
{

}
