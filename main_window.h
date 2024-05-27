#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <qcustomplot.h>
#include <stm_spectrometr.h>
#include "json_utils.h"

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
    QJsonArray m_expositions;

private slots:
    void showPlot(QVector<double>& channels,
                  QVector<double>& values, double max);
    void exit();
    // QWidget interface
    void on_comboBox_expositions_currentIndexChanged(int index);
    void on_pushButton_spectr_toggled(bool checked);

protected:
    void closeEvent(QCloseEvent *event);
};
#endif // MAIN_WINDOW_H
