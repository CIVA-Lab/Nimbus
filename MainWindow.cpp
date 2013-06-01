#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMenu>
#include <QGLFormat>
#include <QDebug>

#include <QVector>
#include <QtCore/qmath.h>

#include <QProgressDialog>
#include <QFileDialog>
#include <QMessageBox.h>

#include "rply.h"

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

// Error callback for rply.  Hides default message handler
static void plyErrorCallback(p_ply ply, const char *message)
{
  // No op
  Q_UNUSED(ply)
  Q_UNUSED(message)
}

static int vertexCallback(p_ply_argument arg)
{
  QVector<float> *points;

  ply_get_argument_user_data(arg, (void **)&points, NULL);
  points->push_back(ply_get_argument_value(arg));

  return 1;
}

int randomInt(int min, int max)
{
  double r = (double)qrand()/RAND_MAX;
  return min + r * (max - min);
}

void MainWindow::openFile()
{
  // Get file to open
  QString path = QFileDialog::getOpenFileName(this, "Open File");

  if(path.isEmpty()) return;

  QVector<float> points;

  // For now only support ply files using rply
  p_ply ply = ply_open(path.toLocal8Bit().constData(), plyErrorCallback, 0,
                       NULL);

  // If open failure
  if(!ply)
  {
    QMessageBox::critical(this, "Unable to open file",
                          "Failed to open " + path);
    return;
  }

  if(!ply_read_header(ply))
  {
    QMessageBox::critical(this, "Unable to open file",
                          path + " is not a PLY file.");
    return;

  }

  // Register read callback
  int numPoints = ply_set_read_cb(ply, "vertex", "x", vertexCallback, &points, 0);
  ply_set_read_cb(ply, "vertex", "y", vertexCallback, &points, 0);
  ply_set_read_cb(ply, "vertex", "z", vertexCallback, &points, 0);

  qDebug() << "Expecting" << numPoints << "vertices.";

  if(!ply_read(ply))
  {
    QMessageBox::critical(this, "Failed reading file", "Failed reading " +
                          path);
    return;
  }

  qDebug() << "Got" << points.count()  << "points!";

  // Shuffle vector using "Knuth Shuffle"
  int vertexCount = points.count()/3;

  for(int i = vertexCount - 1; i > 0; i--)
  {
    int j = randomInt(0, i);

    float tmp = points.at(i * 3 + 0);
    points[i * 3 + 0] = points.at(j * 3 + 0);
    points[j * 3 + 0] = tmp;

    tmp = points.at(i * 3 + 1);
    points[i * 3 + 1] = points.at(j * 3 + 1);
    points[j * 3 + 1] = tmp;

    tmp = points.at(i * 3 + 2);
    points[i * 3 + 2] = points.at(j * 3 + 2);
    points[j * 3 + 2] = tmp;
  }

  m_viewer->setPointModel(points);
}

MainWindow::~MainWindow()
{
    delete ui;
}
