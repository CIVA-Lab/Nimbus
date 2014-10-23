#include <QApplication>
#include <QtPlugin>
#include <QStringList>
#include <QFileInfo>
#include "MainWindow.h"
#include "Viewer.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(Nimbus);

    QApplication a(argc, argv);
#if defined(Q_WS_WIN) || defined(Q_WS_X11)
    // Set application wide icon; needed for static windows builds
    a.setWindowIcon(QIcon(":/Nimbus.ico"));
#endif

#if QT_VERSION >= 0x050100
    // Enable retina icons
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    MainWindow w;
    w.show();

    // Get list of passed-in arguments
    QStringList args = a.arguments();
    if(!args.isEmpty())
    {
      // Remove application path from list (if present)
      QFileInfo info(args.at(0));
      if(info.absoluteFilePath() == a.applicationFilePath())
      {
        args.removeAt(0);
      }
    }

    // Any remaining arguments are paths to open; open first only
    if(!args.isEmpty())
      w.openFile(args.at(0));

    return a.exec();
}
