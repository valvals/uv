#include "main_window.h"
#include "qrc_files_restorer.h"
#include <QApplication>

const char root_dir_name[] = "/experiments";

int main(int argc, char *argv[])
{
    QrcFilesRestorer::restoreFilesFromQrc(":/jsons");
    const QString root_dir_path = QDir::currentPath()+root_dir_name;
    QDir dir(root_dir_path);
    if(!dir.exists(root_dir_path)){
        dir.mkdir(root_dir_path);
    };
    qDebug()<<"--->"<<dir.exists(root_dir_name);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
