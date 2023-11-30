#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Genavir");
    QCoreApplication::setApplicationName("UDP_Diffusion");
    MainWindow w;
    w.show();
    return a.exec();
}
