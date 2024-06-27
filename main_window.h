#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTimer>
#include <qcustomplot.h>
#include <stm_spectrometr.h>
#include "json_utils.h"
#include <QVideoWidget>
#include <QMediaPlayer>

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
    Ui::MainWindow* ui;
    stm_spectrometr* m_stm;
    QTimer m_time_timer;
    QJsonArray m_expositions;
    QJsonArray m_objects;
    QMediaPlayer* m_media_player;
    QVideoWidget* m_video_widget;
    QProcess m_cam_process;
    bool m_is_camera_picture_ready;

private slots:
    void showPlot(QVector<double>& channels,
                  QVector<double>& values, double max);
    void update_camera_image(int);
    void exit();
    // QWidget interface
    void on_comboBox_expositions_currentIndexChanged(int index);
    void on_pushButton_spectr_toggled(bool checked);
    void on_pushButton_record_toggled(bool checked);

protected:
    void closeEvent(QCloseEvent *event);
};
#endif // MAIN_WINDOW_H
