#include <QApplication>
#include <QCommandLineParser>
#include <QtPlugin>
#include <QStringList>
#include <QFileInfo>
#include "MainWindow.h"
#include "Viewer.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(Nimbus);

    QApplication a(argc, argv);
#if defined(Q_WS_WIN) || defined(Q_OS_LINUX)
    // Set application wide icon; needed for static windows builds
    a.setWindowIcon(QIcon(":/Nimbus.ico"));
#endif

#if QT_VERSION >= 0x050100
    // Enable retina icons
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    MainWindow w;
    w.show();

    QCommandLineParser parser;
    parser.process(a);
    QStringList args = parser.positionalArguments();

    if(!args.isEmpty())
      w.openFile(args.first());

    return a.exec();
}
