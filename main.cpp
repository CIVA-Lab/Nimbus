#include <QtGui/QApplication>
#include "MainWindow.h"
#include "Viewer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

//    Viewer v;
//    v.show();

    return a.exec();
}
