#include "main_window.h"
#include "ui_main_window.h"
#include "QDebug"
#include <cstring>
#include <QProcess>
#include <QtConcurrent/QtConcurrentRun>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(480,800);
    this->layout()->setSizeConstraint( QLayout::SetFixedSize );
    m_is_camera_picture_ready = false;
    m_is_record = false;
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
    QStringList modes = {"Небо","Газ"};
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
    on_pushButton_spectr_create_new_experiment_clicked();

    /*QVector<double>values = {1,2,3,4,5};
    QtConcurrent::run([this,values](){
      save_spectr_to_text(values);
    });*/


}

MainWindow::~MainWindow()
{
    delete ui;
    m_cam_process.close();
}

void MainWindow::start_capture_process()
{
    m_is_camera_picture_ready = false;
    m_cam_process.start("libcamera-still --immediate -n  -v 0 --shutter 1000 awbgains=0.3,0.5  -o rp_image.jpg");
    m_capture_img_name = "//"+QDateTime::currentDateTimeUtc().toString("yyyy_MM_dd_hh_mm_ss_z")+".jpg";
}

void MainWindow::save_spectr_to_text(const QVector<double>& values)
{
    QString file_name;
    if(ui->comboBox_mode->currentText()=="Небо"){
        file_name = m_capture_sky_dat_dir;
    }else if(ui->comboBox_mode->currentText()=="Газ"){
        file_name = m_capture_gas_dat_dir;
    }
    auto value = m_expositions[ui->comboBox_expositions->currentIndex()].toObject().value(ui->comboBox_expositions->currentText()).toDouble();
    file_name = file_name+"//"+QDateTime::currentDateTimeUtc().toString("yyyy_MM_dd_hh_mm_ss_z_")+QString::number(value)+".txt";
    QString spectr_data;
    QFile file(file_name);
    file.open(QIODevice::WriteOnly);
    for(int i=0;i<values.size();++i){
        spectr_data.append(QString::number(values[i])+"\n");
    }
    file.write(spectr_data.toLatin1());
    file.close();
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
        QPainter painter(&pm);
        QPen pen;
        pen.setColor(QColor("red"));
        pen.setWidth(3);
        painter.setPen(pen);
        QRect rect;
        rect.setTopLeft(QPoint(1287,900));
        rect.setBottomRight(QPoint(1301,947));
        painter.drawRect(rect);
        painter.drawEllipse(QPoint(1294,923),200,200);
        painter.end();
        QMatrix rm;
        rm.rotate(90);
        pm = pm.transformed(rm);
        ui->label_camera->setPixmap(pm);
        start_capture_process();
    }
    if(m_is_record){
        QtConcurrent::run([this,values](){
            save_spectr_to_text(values);
        });
        //save_spectr_to_text(values);
    }
}

void MainWindow::update_camera_image(int)
{
    m_is_camera_picture_ready = true;
    if(m_is_record){
        QPixmap pm("rp_image.jpg");
        QString dir;
        if(ui->comboBox_mode->currentText()=="Небо"){
            dir = m_capture_sky_img_dir;
        }else if(ui->comboBox_mode->currentText()=="Газ"){
            dir = m_capture_gas_img_dir;
        }
        pm.save(dir+m_capture_img_name);
    }
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
    m_is_record = checked;
    if(checked){
        ui->pushButton_record->setText("stop");
    }else{
        ui->pushButton_record->setText("record");
    }
}

void MainWindow::on_pushButton_expo_minus_clicked()
{
    auto index = ui->comboBox_expositions->currentIndex();
    if(index!=0)
        ui->comboBox_expositions->setCurrentIndex(--index);
}


void MainWindow::on_pushButton_expo_plus_clicked()
{
    auto index = ui->comboBox_expositions->currentIndex();
    if(index!=ui->comboBox_expositions->count()-1)
        ui->comboBox_expositions->setCurrentIndex(++index);
}


void MainWindow::on_pushButton_spectr_create_new_experiment_clicked()
{
    const QString date_time_stamp = QDateTime::currentDateTimeUtc().toString("yyyy_MM_dd_hh_mm_ss_zz_");
    m_current_experiment_dir = QDir::currentPath()+"/experiments/"+date_time_stamp+m_objects[ui->comboBox_objects->currentIndex()].toObject()["alias"].toString();
    QDir dir;
    dir.mkdir(m_current_experiment_dir);
    m_capture_gas_dat_dir = m_current_experiment_dir + "/gas_dat";
    m_capture_gas_img_dir = m_current_experiment_dir + "/gas_img";
    m_capture_sky_dat_dir = m_current_experiment_dir + "/sky_dat";
    m_capture_sky_img_dir = m_current_experiment_dir + "/sky_img";
    dir.mkdir(m_capture_sky_img_dir);
    dir.mkdir(m_capture_gas_img_dir);
    dir.mkdir(m_capture_sky_dat_dir);
    dir.mkdir(m_capture_gas_dat_dir);
}

