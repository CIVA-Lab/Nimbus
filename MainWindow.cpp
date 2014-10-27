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

#include <QtCore/qmath.h>

#include <QMimeData>

#include "rply.h"

#include "PointCloud.h"
#include "PLYLoader.h"
#include "PointGenerator.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
  ui(new Ui::MainWindow), m_viewer(NULL)
{
    ui->setupUi(this);

    // Get default OpenGL format
    QGLFormat format = QGLFormat::defaultFormat();

    // Optional sync to VBLANK; enabled by default
#ifndef NO_VBLANK_SYNC
    format.setSwapInterval(1);
#endif

    // Enable sample buffers
    format.setOption(QGL::SampleBuffers);
    // Request alpha buffer
    format.setAlpha(true);
    // Request stereo
    format.setStereo(true);

    // Create OpenGL context
    QGLContext context(format);
    // Try to create context with format
    if(!context.create())
    {
      // If context couldn't be created, disable stereo buffers
      format.setStereo(false);
    }

    // Update default OpenGL format
    QGLFormat::setDefaultFormat(format);

    // Create viewer; don't set as the central widget until later. This
    // suppresses an "Invalid Drawable" message for OS X
    m_viewer = new Viewer();

    m_createOptions = new CreatePointCloudDialog(this);
    QMenu *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction("Open File...", this, SLOT(openFile()),
                        QKeySequence::Open);
    fileMenu->addAction("Create Point Cloud...", m_createOptions,
                        SLOT(show()));
    connect(m_createOptions, SIGNAL(accepted(QString,int,bool)),
            SLOT(createPointCloud(QString,int,bool)));

    // Hide option to export movie from flight path; Not ready for other users
    // fileMenu->addAction("Save Path Movie", m_viewer, SLOT(savePathMovie()));

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
    connect(m_viewer, SIGNAL(fastInteractionChanged(bool)),
            m_displayOptions, SLOT(setFastInteraction(bool)));


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
    connect(m_displayOptions, SIGNAL(fastInteractionChanged(bool)),
            m_viewer, SLOT(setFastInteraction(bool)));

    m_displayOptions->setMultisampleAvailable(m_viewer->multisampleAvailable());

    QMenu *displayMenu = menuBar()->addMenu("Display");
    displayMenu->addAction("Display Options...", m_displayOptions,
                           SLOT(show()));

    // Create stereo options dialog
    m_stereoOptions = new StereoOptionsDialog(this);
    m_stereoOptions->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    // Sync viewer and dialog
    connect(m_stereoOptions, &StereoOptionsDialog::IODistanceChanged,
            m_viewer, &Viewer::setIODistance);
    connect(m_stereoOptions, &StereoOptionsDialog::screenWidthChanged,
            m_viewer, &Viewer::setScreenWidth);
    connect(m_stereoOptions, &StereoOptionsDialog::focusDistanceChanged,
            m_viewer, &Viewer::setFocusDistance);
    connect(m_stereoOptions, &StereoOptionsDialog::swapLeftRightChanged,
            m_viewer, &Viewer::setSwapLeftRight);

    // Sync viewer values back to dialog
    connect(m_viewer, &Viewer::focusDistanceChanged, m_stereoOptions,
            &StereoOptionsDialog::setFocusDistance);
    connect(m_viewer, &Viewer::physicalScreenWidthChanged, m_stereoOptions,
            &StereoOptionsDialog::setScreenWidth);

    m_stereoOptions->setIODistance(m_viewer->camera()->IODistance());
    m_stereoOptions->setScreenWidth(m_viewer->camera()->physicalScreenWidth());
    m_stereoOptions->setFocusDistance(m_viewer->camera()->focusDistance());

    // Create menu item
    displayMenu->addAction("Stereo Options...", m_stereoOptions, SLOT(show()));

    QMenu *helpMenu = menuBar()->addMenu("Help");
    helpMenu->addAction("Help...", m_viewer, SLOT(help()));

    // Set default viewer values
    m_viewer->setPointSize(1);
    m_viewer->setSmoothPoints(true);
    m_viewer->setColorPoints(true);
    m_viewer->setDepthMasking(true);
    m_viewer->setMultisample(true);
    m_viewer->setFastInteraction(false);
    m_viewer->setPointDensity(100);

    // Set viewer as central widget
    setCentralWidget(m_viewer);

    resize(825,550);

    setAcceptDrops(true);
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

      // Load cameras
      for(int i = 0; i < loader.cameraPositions().count(); i += 3)
      {
        Vec position(loader.cameraPositions().at(i + 0),
                     loader.cameraPositions().at(i + 1),
                     loader.cameraPositions().at(i + 2));

        m_viewer->camera()->setPosition(position);

        Vec up(loader.cameraUpVectors().at(i + 0),
               loader.cameraUpVectors().at(i + 1),
               loader.cameraUpVectors().at(i + 2));

        m_viewer->camera()->setUpVector(up);

        Vec aim(loader.cameraAimVectors().at(i + 0),
                loader.cameraAimVectors().at(i + 1),
                loader.cameraAimVectors().at(i + 2));
        m_viewer->camera()->setViewDirection(aim);

        Vec aspect(loader.cameraAspectRatios().at(i + 0),
                   loader.cameraAspectRatios().at(i + 1),
                   loader.cameraAspectRatios().at(i + 2));

        m_viewer->camera()->setFieldOfView(2 * qAtan((0.5 * aspect.y)/aspect.z));

        m_viewer->m_fov.push_back(2 * qAtan((0.5 * aspect.y)/aspect.z));
        m_viewer->camera()->addKeyFrameToPath(1);
      }

      // Check that keyframes have been added to path 1
      if(m_viewer->camera()->keyFrameInterpolator(1) != NULL)
      {
        connect(m_viewer->camera()->keyFrameInterpolator(1),
                SIGNAL(interpolated()), m_viewer, SLOT(updateGL()));

        m_viewer->camera()->keyFrameInterpolator(1)->setInterpolationSpeed(4.0);
      }
      progress.close();

      return;
    }
  }

  QMessageBox::critical(this, "Unable to open file",
                        path + " is not a supported format.");
}

bool MainWindow::canRead(const QString &path)
{
  return PLYLoader::canRead(path);
}

void MainWindow::createPointCloud(QString shape, int count, bool asSurface)
{
  PointGenerator generator;
  QProgressDialog progress(this);
  progress.setWindowModality(Qt::WindowModal);

  connect(&generator, SIGNAL(setRange(int,int)), &progress,
          SLOT(setRange(int,int)));
  connect(&generator, SIGNAL(progress(int)), &progress, SLOT(setValue(int)));
  connect(&progress, SIGNAL(canceled()), &generator, SLOT(cancel()));

  PointCloud cloud = generator.createPointCloud(shape, count, asSurface);

  progress.hide();

  m_viewer->setPointCloud(cloud);
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
        // Force copy action (for correct mouse pointer icon)
        e->setDropAction(Qt::CopyAction);
        e->accept();
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
