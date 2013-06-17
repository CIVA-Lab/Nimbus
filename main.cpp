#include <QtGui/QApplication>
#include <QtPlugin>
#include "MainWindow.h"
#include "Viewer.h"

// Needed for static windows build
#ifdef NIMBUS_STATIC
Q_IMPORT_PLUGIN(qico)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // Needed to set icon for windows
    Q_INIT_RESOURCE(Nimbus);
    w.setWindowIcon(QIcon(":/Nimbus.ico"));

    return a.exec();
}
