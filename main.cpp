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
    Q_INIT_RESOURCE(Nimbus);

    QApplication a(argc, argv);
    // Set application wide icon; needed for static windows builds
    a.setWindowIcon(QIcon(":/Nimbus.ico"));

    MainWindow w;
    w.show();

    return a.exec();
}
