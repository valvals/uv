#include "main_window.h"
#include "qrc_files_restorer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QrcFilesRestorer::restoreFilesFromQrc(":/jsons");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
