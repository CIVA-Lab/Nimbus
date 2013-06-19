#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMenu>
#include <QGLFormat>
#include <QDebug>

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>

#include <QVector>
#include <QtCore/qmath.h>

#include <QProgressDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "rply.h"

#include "PointCloud.h"
#include "PLYLoader.h"
#include "LASLoader.h"

class Sphere
{
public:
  Sphere(double radius = 0.5) : m_radiusSquared(radius*radius) { }
  double operator()(double x, double y, double z)
  {
    return x*x + y*y + z*z - m_radiusSquared;
  }
private:
  double m_radiusSquared;
};

class Cylinder
{
public:
  Cylinder(double radius = 0.5) : m_radiusSquared(radius*radius) { }
  double operator()(double x, double y, double z)
  {
    Q_UNUSED(z)
    return x*x + y*y - m_radiusSquared;
  }
private:
  double m_radiusSquared;
};

class Cube
{
public:
  Cube(double width = 1.0) : m_halfWidth(width/2.0) { }
  double operator()(double x, double y, double z)
  {
    return qMax(qMax(qAbs(x), qAbs(y)), qAbs(z)) - m_halfWidth;
  }
private:
  double m_halfWidth;
};

class Torus
{
public:
  Torus(double radius = 0.3, double height = 0.2) :
    m_majorRadius(radius),
    m_majorRadiusSquared(m_majorRadius*m_majorRadius),
    m_minorRadiusSquared(height*height) { }
  double operator()(double x, double y, double z)
  {
    return (x*x + y*y + z*z + m_majorRadiusSquared - m_minorRadiusSquared)
        * (x*x + y*y + z*z + m_majorRadiusSquared - m_minorRadiusSquared)
        - 4 * m_majorRadiusSquared * (x*x + y*y);
  }

private:
  double m_majorRadius;
  double m_majorRadiusSquared;
  double m_minorRadiusSquared;
};

template <typename Shape>
QVector<float> MainWindow::GeneratePointVolume(int count, Shape shape)
{
  QVector<float> result;
  double x, y, z;

  QProgressDialog progress("Generating points...", "Cancel", 0, count, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(1000);

  int step = qMax(1.0, count/100.0);

  for(int i = 0; i < count; ++i)
  {
    do {
      x = (double)qrand()/RAND_MAX - 0.5;
      y = (double)qrand()/RAND_MAX - 0.5;
      z = (double)qrand()/RAND_MAX - 0.5;
    } while (shape(x,y,z) > 0.0);

    result.push_back(x);
    result.push_back(y);
    result.push_back(z);

    if(i % step == 0)
    {
      progress.setValue(i);
    }

    if(progress.wasCanceled())
      break;
  }

  progress.setValue(count);

  return result;
}

template <typename Shape>
QVector<float> MainWindow::GeneratePointSurface(int count, Shape shape, double delta)
{
  QVector<float> result;
  double x, y, z;

  QProgressDialog progress("Generating points...", "Cancel", 0, count, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(1000);

  int step = qMax(1.0, count/100.0);

  for(int i = 0; i < count; ++i)
  {
    do {
      x = (double)qrand()/RAND_MAX - 0.5;
      y = (double)qrand()/RAND_MAX - 0.5;
      z = (double)qrand()/RAND_MAX - 0.5;
    } while (qAbs(shape(x,y,z)) > delta);

    result.push_back(x);
    result.push_back(y);
    result.push_back(z);

    if(i % step == 0)
    {
      progress.setValue(i);
    }

    if(progress.wasCanceled())
      break;
  }

  progress.setValue(count);

  return result;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
  ui(new Ui::MainWindow), m_viewer(NULL)
{
    ui->setupUi(this);

    // Get default OpenGL format
    QGLFormat format = QGLFormat::defaultFormat();
    // Enable sync to VBLANK
    format.setSwapInterval(1);
    // Enable sample buffers
    format.setOption(QGL::SampleBuffers);
    // Request alpha buffer
    format.setAlpha(true);
    // Update default OpenGL format
    QGLFormat::setDefaultFormat(format);

    m_viewer = new Viewer();
    setCentralWidget(m_viewer);

    m_createOptions = new CreatePointCloudDialog(this);
    QMenu *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction("Open File...", this, SLOT(openFile()),
                        QKeySequence::Open);
    fileMenu->addAction("Create Point Cloud...", m_createOptions,
                        SLOT(show()));
    connect(m_createOptions, SIGNAL(accepted(QString,int,bool)),
            SLOT(createPointCloud(QString,int,bool)));
    fileMenu->addAction("Quit", qApp, SLOT(quit()));

    m_displayOptions = new DisplayOptionsDialog(this);
    m_displayOptions->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    // Sync viewer settings to display options dialog
    connect(m_viewer, SIGNAL(pointSizeChanged(int)),
            m_displayOptions, SLOT(setPointSize(int)));
    connect(m_viewer, SIGNAL(pointDensityChanged(int)),
            m_displayOptions, SLOT(setPointDensity(int)));
    connect(m_viewer, SIGNAL(smoothPointsChanged(bool)),
            m_displayOptions, SLOT(setSmoothPoints(bool)));
    connect(m_viewer, SIGNAL(pointColorChanged(bool)),
            m_displayOptions, SLOT(setPointColor(bool)));
    connect(m_viewer, SIGNAL(depthMaskingChanged(bool)),
            m_displayOptions, SLOT(setDepthMask(bool)));
    connect(m_viewer, SIGNAL(multisampleChanged(bool)),
            m_displayOptions, SLOT(setMultisample(bool)));


    // Sync display options dialog to viewer
    connect(m_displayOptions, SIGNAL(pointSizeChanged(int)),
            m_viewer, SLOT(setPointSize(int)));
    connect(m_displayOptions, SIGNAL(pointDensityChanged(int)),
            m_viewer, SLOT(setPointDensity(int)));
    connect(m_displayOptions, SIGNAL(pointSmoothChanged(bool)),
            m_viewer, SLOT(setSmoothPoints(bool)));
    connect(m_displayOptions, SIGNAL(pointColorChanged(bool)),
            m_viewer, SLOT(setColorPoints(bool)));
    connect(m_displayOptions, SIGNAL(pointDepthChanged(bool)),
            m_viewer, SLOT(setDepthMasking(bool)));
    connect(m_displayOptions, SIGNAL(multiSampleChanged(bool)),
            m_viewer, SLOT(setMultisample(bool)));

    m_displayOptions->setMultisampleAvailable(m_viewer->multisampleAvailable());

    QMenu *displayMenu = menuBar()->addMenu("Display");
    displayMenu->addAction("Display Options...", m_displayOptions,
                           SLOT(show()));


    QMenu *helpMenu = menuBar()->addMenu("Help");
    helpMenu->addAction("Help...", m_viewer, SLOT(help()));

    // Set default viewer values
    m_viewer->setPointSize(1);
    m_viewer->setSmoothPoints(true);
    m_viewer->setColorPoints(true);
    m_viewer->setDepthMasking(true);
    m_viewer->setMultisample(true);
    m_viewer->setPointDensity(100);

    resize(800,600);

    setAcceptDrops(true);
}

void MainWindow::loadSphereVolume(int pointCount)
{
  m_viewer->setPointModel(GeneratePointVolume(pointCount, Sphere()));
}

void MainWindow::createPointCloud(QString shape, int count, bool asSurface)
{
  QVector<float> points;

  float delta = 0.001;

  if(shape == "Cube")
  {
    if(!asSurface)
      points = GeneratePointVolume(count, Cube());
    else
      points = GeneratePointSurface(count, Cube(1.0 - delta), delta);
  }
  if(shape == "Sphere")
  {
    if(!asSurface)
      points = GeneratePointVolume(count, Sphere());
    else
      points = GeneratePointSurface(count, Sphere(0.5 - delta), delta);
  }
  if(shape == "Torus")
  {
    if(!asSurface)
      points = GeneratePointVolume(count, Torus());
    else
      points = GeneratePointSurface(count, Torus(0.3 - delta, 0.2 - delta), delta);
  }
  if(shape == "Cylinder")
  {
    if(!asSurface)
      points = GeneratePointVolume(count, Cylinder());
    else
      points = GeneratePointSurface(count, Cylinder(0.5 - delta), delta);
  }

  m_viewer->setPointModel(points);
}

void MainWindow::openFile()
{
  // Get file to open
  QString path = QFileDialog::getOpenFileName(this, "Open File");

  if(!path.isEmpty())
    openFile(path);
}

void MainWindow::openFile(const QString &path)
{
  // Try to open PLY file
  if(PLYLoader::canRead(path))
  {
    PLYLoader loader;
    if(loader.open(path))
    {
      QProgressDialog progress(this);
      progress.setWindowModality(Qt::WindowModal);
      progress.setRange(0, 100);
      progress.setMinimumDuration(1000);
      progress.setAutoClose(false);

      connect(&loader, SIGNAL(progress(int)), &progress, SLOT(setValue(int)));
      connect(&progress, SIGNAL(canceled()), &loader, SLOT(cancel()));

      PointCloud cloud = loader.load();
      cloud.shuffle();
      m_viewer->setPointCloud(cloud);

      progress.close();

      return;
    }
  }

  if(LASLoader::canRead(path))
  {
    LASLoader loader;
    if(loader.open(path))
    {
      QProgressDialog progress(this);
      progress.setWindowModality(Qt::WindowModal);
      progress.setRange(0, 100);
      progress.setMinimumDuration(1000);

      connect(&loader, SIGNAL(progress(int)), &progress, SLOT(setValue(int)));
      connect(&progress, SIGNAL(canceled()), &loader, SLOT(cancel()));

      PointCloud cloud = loader.load();
      cloud.shuffle();
      m_viewer->setPointCloud(cloud);

      return;
    }
  }

  QMessageBox::critical(this, "Unable to open file",
                        path + " is not a supported format.");
}

bool MainWindow::canRead(const QString &path)
{
  return LASLoader::canRead(path) || PLYLoader::canRead(path);
}

void MainWindow::closeEvent(QCloseEvent *)
{
  qApp->quit();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
  if(e->mimeData()->hasUrls())
  {
    if(e->mimeData()->urls().count() == 1)
    {
      // See if file can be read
      QString path = e->mimeData()->urls().at(0).toLocalFile();

      if(canRead(path))
      {
        e->acceptProposedAction();
      } else {
        e->ignore();
      }
    }
  }
}

void MainWindow::dropEvent(QDropEvent *e)
{
  if(e->mimeData()->hasUrls())
  {
    if(e->mimeData()->urls().count() == 1)
    {
      // See if file can be read
      QString path = e->mimeData()->urls().at(0).toLocalFile();

      e->accept();

      // The following queues the opening of the file by invoking openFile as
      // a queued slot call.  This causes the dropEvent to be handled
      // immediately, and the file to be opened on return to the event loop.
      // Keeps the mouse pointer from becoming a wait pointer and makes the
      // gui more responsive.
      QMetaObject::invokeMethod(this, "openFile", Qt::QueuedConnection,
                                Q_ARG(QString, path));
    }
  }
}

MainWindow::~MainWindow()
{
    delete ui;
}
