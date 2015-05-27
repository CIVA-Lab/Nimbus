#include "Viewer.h"
#include <QDebug>
#include <QVector3D>
#include <QtCore/qmath.h>
#include <QAction>
#include <QGLShaderProgram>
#include <QGLShader>
#include <QKeyEvent>
// For pi constant
#include <cmath>

using namespace qglviewer;

// Necessary for older windows OpenGL
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

Viewer::Viewer(QWidget *parent) :
  QGLViewer(parent),
  m_vertexCount(0),
  m_density(1.0),
  m_pointSize(1.0),
  m_smoothPoints(true),
  m_fastInteraction(false),
  m_fastInteractionMax(250000),
  m_swapLeftRight(false),
  m_stereo(false),
  m_showLogo(true),
  m_turntableRPM(1.0),
  m_turntableStarted(false)
{
  setAutoFillBackground(false);
  setKeyDescription(Qt::Key_P, "Toggle smooth points");
  setKeyDescription(Qt::Key_R, "Restore default view");
  setKeyDescription(Qt::Key_T, "Toggle turntable animation");
  setKeyDescription(Qt::Key_T + Qt::SHIFT, "Reset turntable");
  setKeyDescription(Qt::Key_Minus, "Decrease turntable speed");
  setKeyDescription(Qt::Key_Plus, "Increase turntable speed");
  setShortcut(EXIT_VIEWER, 0);

  // Load logo pixmap at 150x172.  QIcon will return a retina quality version
  //  pixmap if available
  m_logoPixmap = QIcon(":/MULogo.png").pixmap(150,172);

  // Install manipulated frame
  setManipulatedFrame(new ManipulatedFrame());

  // Set default stereo mode; use hardware when available
  if(supportsHardwareStereo())
    setStereoMode(Hardware);
  else
    setStereoMode(Red_Cyan);
}

bool Viewer::setPointCloud(const PointCloud &cloud)
{
  if(!bindToVertexBuffer(cloud.pointData()))
    return false;

  if(cloud.hasColor())
  {
    if(!loadColorsToBuffer(cloud.colorDataF()))
    {
      qDebug() << "Failed loading color data.";
      return false;
    }
  } else {
    if(m_colorBuffer.isCreated())
    {
      m_colorBuffer.destroy();
    }
  }

  QVector3D min = cloud.boundingBoxMinimum();
  QVector3D max = cloud.boundingBoxMaximum();

  setSceneBoundingBox(qglviewer::Vec(min.x(), min.y(), min.z()),
                      qglviewer::Vec(max.x(), max.y(), max.z()));

  showEntireScene();
  setPointDensity(100);

  notifyStereoParametersChanged();

  return true;
}

bool Viewer::multisampleAvailable() const
{
  return format().testOption(QGL::SampleBuffers);
}

bool Viewer::supportsHardwareStereo() const
{
  return format().stereo();
}

void Viewer::setPointSize(int pointSize)
{
  // Validate value change to prevent signal/slot cycles
  if(pointSize != m_pointSize)
  {
    m_pointSize = qBound(1.0, (double)pointSize, 10.0);
    displayMessage(QString("Point Size %1").arg(m_pointSize));
    emit pointSizeChanged(m_pointSize);
    update();
  }
}

void Viewer::setPointDensity(int density)
{
  // Ensure changed value to prevent signal/slot cycles
  if(density != m_density)
  {
    m_density = qBound(1.0, (double)density, 100.0);
    displayMessage(QString("%L1 Points").arg((int)(m_vertexCount * m_density/100.0)));
    emit pointDensityChanged(m_density);
    update();
  }
}

void Viewer::setSmoothPoints(bool smoothPoints)
{
  makeCurrent();
  if(smoothPoints)
  {
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    displayMessage("Smooth Points On");
  } else {
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_BLEND);
    displayMessage("Smooth Points Off");
  }

  m_smoothPoints = smoothPoints;

  emit smoothPointsChanged(smoothPoints);
  update();
}

void Viewer::toggleSmoothPoints()
{
  setSmoothPoints(!m_smoothPoints);
}

void Viewer::setColorPoints(bool value)
{
  m_colorPoints = value;
  emit pointColorChanged(value);
  update();
}

void Viewer::setDepthMasking(bool value)
{
  if(m_depthMasking != value)
  {
    m_depthMasking = value;
    if(m_depthMasking)
      displayMessage("Depth mask enabled");
    else
      displayMessage("Depth mask disabled");

    emit depthMaskingChanged(value);

    update();
  }
}

void Viewer::setMultisample(bool value)
{
  if(m_multisample != value)
  {
    m_multisample = value;

    makeCurrent();
    if(m_multisample)
    {
      glEnable(GL_MULTISAMPLE);
      displayMessage("Multisampling enabled");
    } else {
      glDisable(GL_MULTISAMPLE);
      displayMessage("Multisampling disabled");
    }
    emit multisampleChanged(m_multisample);

    update();
  }
}

void Viewer::setFastInteraction(bool value)
{
  if(m_fastInteraction != value)
  {
    m_fastInteraction = value;
    if(m_fastInteraction)
      displayMessage("Fast Interaction enabled");
    else
      displayMessage("Fast Interaction disabled");

    emit fastInteractionChanged(m_fastInteraction);
  }
}

void Viewer::restoreView()
{
  // Restore default view by creating new camera and fitting scene
  setCamera(new Camera());
  showEntireScene();

  // This changes some parameters related to stereo; force refresh
  notifyStereoParametersChanged();
}

void Viewer::setIODistance(double distance)
{
  camera()->setIODistance((float) distance);
  update();
}

void Viewer::setScreenWidth(double width)
{
  camera()->setPhysicalScreenWidth((float) width);
  update();
}

void Viewer::setFocusDistance(double distance)
{
  camera()->setFocusDistance((float) distance);
  update();
}

void Viewer::setSwapLeftRight(bool swap)
{
  m_swapLeftRight = swap;
  update();
}

void Viewer::setStereo(bool stereo)
{
  m_stereo = stereo;

  Q_EMIT(stereoChanged(m_stereo));

  update();
}

void Viewer::toggleStereo()
{
  setStereo(!stereoEnabled());
}

void Viewer::setStereoMode(Viewer::StereoMode mode)
{
  if(m_stereoMode != mode)
  {
    m_stereoMode = mode;
    emit stereoModeChanged(m_stereoMode);
    update();
  }
}

void Viewer::init()
{
#ifdef DEBUG_GL_CAPABILITIES
  // See if shader programs are available
  if(QGLShaderProgram::hasOpenGLShaderPrograms())
  {
    qDebug() << "Shader programs supported!";
  }

  qDebug() << "Depth size" << format().depthBufferSize();
  qDebug() << "Alpha size" << format().alphaBufferSize();
  qDebug() << "Red size" << format().redBufferSize();
  GLint redBits;
  glGetIntegerv(GL_RED_BITS, &redBits);
  qDebug() << "Red bits" << redBits;
  qDebug() << "Multisample" << format().testOption(QGL::SampleBuffers);
  qDebug() << "OpenGL Version flags" << format().openGLVersionFlags();
  qDebug().nospace() << "OpenGL Version " << format().majorVersion() << "."
           << format().minorVersion();
  qDebug() << "OpenGL profile" << format().profile();
  qDebug() << "OpenGL vendor string"
           << QLatin1String(reinterpret_cast<const char*>(glGetString(GL_VERSION)));

  qDebug() << "Shader programs:" << QGLShaderProgram::hasOpenGLShaderPrograms();
  qDebug() << "Vertex shader:" << QGLShader::hasOpenGLShaders(QGLShader::Vertex);
  qDebug() << "Geometry shader:" << QGLShader::hasOpenGLShaders(QGLShader::Geometry);
  qDebug() << "Fragment shader:" << QGLShader::hasOpenGLShaders(QGLShader::Fragment);
#endif

  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glDisable(GL_LIGHTING);

  // Bind logo to texture
  m_logoTextureId = bindTexture(m_logoPixmap);

  qglClearColor(QColor(51,51,51,255));
}

void Viewer::draw()
{
  // Apply turntable frame
  glPushMatrix();
  glMultMatrixd(manipulatedFrame()->matrix());

  glPointSize(m_pointSize);

  if(!m_depthMasking)
    glDepthMask(GL_FALSE);

  if(m_multisample)
    glEnable(GL_MULTISAMPLE);

  glEnableClientState(GL_VERTEX_ARRAY);

  if(m_colorPoints)
    glEnableClientState(GL_COLOR_ARRAY);

  if(!m_colorBuffer.isCreated())
  {
    m_vertexBuffer.bind();
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColorPointer(3, GL_FLOAT, 0, 0);
    m_vertexBuffer.release();
  } else {
    m_vertexBuffer.bind();
    glVertexPointer(3, GL_FLOAT, 0, 0);
    m_vertexBuffer.release();

    m_colorBuffer.bind();
    glColorPointer(3, GL_FLOAT, 0, 0);
    m_colorBuffer.release();
  }

  glDrawArrays(GL_POINTS, 0, m_vertexCount * m_density/100.0);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  glDepthMask(GL_TRUE);

  // Restore transforms
  glPopMatrix();
}

void Viewer::drawRedCyanStereo()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  camera()->loadProjectionMatrixStereo(!m_swapLeftRight);
  camera()->loadModelViewMatrixStereo(!m_swapLeftRight);

  glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);

  draw();

  glClear(GL_DEPTH_BUFFER_BIT);

  glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);

  camera()->loadProjectionMatrixStereo(m_swapLeftRight);
  camera()->loadModelViewMatrixStereo(m_swapLeftRight);

  draw();

  // Restore color mask
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void Viewer::drawRedBlueStereo()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  camera()->loadProjectionMatrixStereo(!m_swapLeftRight);
  camera()->loadModelViewMatrixStereo(!m_swapLeftRight);

  glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);

  draw();

  glClear(GL_DEPTH_BUFFER_BIT);

  glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);

  camera()->loadProjectionMatrixStereo(m_swapLeftRight);
  camera()->loadModelViewMatrixStereo(m_swapLeftRight);

  draw();

  // Restore color mask
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

}

void Viewer::drawSideBySideStereo()
{
  // Get viewport
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);

  // Set left side viewport
  glViewport(vp[0], vp[1], vp[2]/2.0, vp[3]);
  // Load left eye transforms
  camera()->loadProjectionMatrixStereo(!m_swapLeftRight);
  camera()->loadModelViewMatrixStereo(!m_swapLeftRight);

  // Draw
  draw();

  // Set right side viewport
  glViewport(vp[0] + vp[2]/2.0, vp[1], vp[2]/2.0, vp[3]);
  // Load right eye transforms
  camera()->loadProjectionMatrixStereo(m_swapLeftRight);
  camera()->loadModelViewMatrixStereo(m_swapLeftRight);

  // Draw
  draw();

  // Restore viewport
  glViewport(vp[0], vp[1], vp[2], vp[3]);
}

void Viewer::drawHardwareStereo()
{
  // Clear left buffer
  glDrawBuffer(GL_BACK_LEFT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set left eye transforms
  camera()->loadProjectionMatrixStereo(!m_swapLeftRight);
  camera()->loadModelViewMatrixStereo(!m_swapLeftRight);

  // Draw
  draw();

  // Clear right buffer
  glDrawBuffer(GL_BACK_RIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set right eye transforms
  camera()->loadProjectionMatrixStereo(m_swapLeftRight);
  camera()->loadModelViewMatrixStereo(m_swapLeftRight);

  // Draw
  draw();
 }

void Viewer::postDraw()
{
  // Call base class post draw
  QGLViewer::postDraw();

  // Allow logo to be disabled
  if(m_showLogo)
  {
    // Save OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Set matrices for screen drawing
    startScreenCoordinatesSystem();

    // Alpha blending needed for transparency
    glEnable(GL_BLEND);

    // Account for retina sized pixmap
    QSize logoSize = m_logoPixmap.size()/m_logoPixmap.devicePixelRatio();

    QRectF target(QPointF(width() - logoSize.width(),
                          height() - logoSize.height()), logoSize);

    // Draw texture in lower right
    drawTexture(target, m_logoTextureId);

    // Revert matrices
    stopScreenCoordinatesSystem();

    // Restore OpenGL state
    glPopAttrib();
  }
}

void Viewer::fastDraw()
{
  if(!m_fastInteraction)
  {
    draw();
    return;
  }

  int pointsToDraw = qMin((int)(m_vertexCount * m_density/100.0),
                          m_fastInteractionMax);

//  qDebug() << "Fast draw points" << pointsToDraw;
  glPointSize(m_pointSize);

  if(!m_depthMasking)
    glDepthMask(GL_FALSE);

  if(m_multisample)
    glEnable(GL_MULTISAMPLE);

  glEnableClientState(GL_VERTEX_ARRAY);

  if(m_colorPoints)
    glEnableClientState(GL_COLOR_ARRAY);

  if(!m_colorBuffer.isCreated())
  {
    m_vertexBuffer.bind();
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glColorPointer(3, GL_FLOAT, 0, 0);
    m_vertexBuffer.release();
  } else {
    m_vertexBuffer.bind();
    glVertexPointer(3, GL_FLOAT, 0, 0);
    m_vertexBuffer.release();

    m_colorBuffer.bind();
//    glColorPointer(3, GL_UNSIGNED_BYTE, 0, 0);
    // Modified to use floats for color; testing effect on performance
    glColorPointer(3, GL_FLOAT, 0, 0);
    m_colorBuffer.release();
  }

//  glDrawArrays(GL_POINTS, 0, m_vertexCount * 25/100.0);
  glDrawArrays(GL_POINTS, 0, pointsToDraw);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  glDepthMask(GL_TRUE);

}

void Viewer::paintGL()
{
  if(stereoEnabled())
  {
    switch(m_stereoMode)
    {
      case Hardware:
        drawHardwareStereo(); break;
      case Red_Cyan:
        drawRedCyanStereo(); break;
      case Red_Blue:
        drawRedBlueStereo(); break;
      case Side_by_Side:
        drawSideBySideStereo(); break;
    }
  }
  else
  {
    // Clears screen, set model view matrix...
    preDraw();
    // Used defined method. Default calls draw()
    if(camera()->frame()->isManipulated())
      fastDraw();
    else
      draw();
    // Add visual hints: axis, camera, grid...
    postDraw();
  }
  Q_EMIT drawFinished(true);

}

bool Viewer::bindToVertexBuffer(const QVector<float> &vertices)
{
  // Ensure number of vertices is divisible by 3
  if(vertices.count() % 3)
    return false;

  // Make OpenGL context current
  makeCurrent();

  // Destroy currently allocated VBO
  if(m_vertexBuffer.isCreated())
    m_vertexBuffer.destroy();

  // Create VBO
  if(!m_vertexBuffer.create())
  {
    // Trigger error and return
    return false;
  }

  // Bind VBO
  if(!m_vertexBuffer.bind())
  {
    return false;
  }

  // Load VBO
  m_vertexBuffer.allocate(vertices.constData(),
                          vertices.count() * sizeof(float));

  m_vertexCount = vertices.count()/3;

  return glGetError() == GL_NO_ERROR;
}

bool Viewer::loadColorsToBuffer(const QVector<float> &colors)
{
  // Ensure RGB
  if(colors.count() % 3)
    return false;

  // Make context current
  makeCurrent();

  if(m_colorBuffer.isCreated())
    m_colorBuffer.destroy();

  if(!m_colorBuffer.create())
    return false;

  if(!m_colorBuffer.bind())
    return false;

  m_colorBuffer.allocate(colors.constData(),
                          colors.count() * sizeof(float));

  return glGetError() == GL_NO_ERROR;
}

void Viewer::notifyStereoParametersChanged()
{
  emit IODistanceChanged((double) camera()->IODistance());
  emit focusDistanceChanged((double) camera()->focusDistance());
  emit physicalScreenWidthChanged((double) camera()->physicalScreenWidth());
}

void Viewer::keyPressEvent(QKeyEvent *e)
{
  switch(e->key())
  {
  case Qt::Key_P:
    toggleSmoothPoints(); break;
  case Qt::Key_Escape:
    if(isFullScreen()) setFullScreen(false); break;
  case Qt::Key_R:
    restoreView(); break;
  case Qt::Key_S:
    toggleStereo(); break;
  case Qt::Key_T:
    // 't' key
    if(e->modifiers() == Qt::NoModifier)
      toggleTurntable();
    // Shift-T
    else if(e->modifiers() == Qt::ShiftModifier)
      resetTurntable();
    break;
  case Qt::Key_Plus:
  case Qt::Key_Equal:
    increaseTurntableSpeed(); break;
  case Qt::Key_Minus:
    decreaseTurntableSpeed(); break;
  case Qt::Key_At:
    // Alt-@
    if(e->modifiers() == (Qt::AltModifier | Qt::ShiftModifier))
      toggleLogo();
    break;
  // For Qt 5 on OS X, Alt-@ returns the euro symbol (€) 0x20AC
  case 0x20AC:
    toggleLogo();
    break;
  default:
    QGLViewer::keyPressEvent(e);
  }
}

void Viewer::toggleTurntable()
{
  if(manipulatedFrame()->isSpinning())
  {
    // Just pause turntable
    manipulatedFrame()->stopSpinning();
    update();
    displayMessage("Turntable paused");
  } else {
    // If turntable not initialized, start
    if(!m_turntableStarted)
    {
      // Choose up axis
      int whichAxis = 0;
      Vec up = camera()->upVector();
      for(int i = 1; i < 3; ++i)
      {
        if(qAbs(up[i]) > qAbs(up[whichAxis]))
          whichAxis = i;
      }
      m_turntableUp.setValue(0,0,0);
      m_turntableUp[whichAxis] = 1.0;

      // Create quaternion; rotation provided in updateSpin()
      manipulatedFrame()->setSpinningQuaternion(Quaternion(m_turntableUp, 0.0));
      manipulatedFrame()->startSpinning(1000.0/30.0);
      connect(manipulatedFrame(), SIGNAL(spun()), this, SLOT(updateSpin()));
      m_turntableStarted = true;
    }
    // Start turntable moving
    manipulatedFrame()->startSpinning(1000.0/30.0);

    displayMessage("Turntable playing");
  }
}

void Viewer::resetTurntable()
{
  // Get old frame
  ManipulatedFrame* old = manipulatedFrame();
  // Set new frame
  setManipulatedFrame(new ManipulatedFrame());
  // delete old frame
  delete old;
  // Disconnect spin updates
  disconnect(manipulatedFrame(), SIGNAL(spun()), this, SLOT(updateSpin()));

  m_turntableStarted = false;

  displayMessage("Turntable reset");
}

void Viewer::increaseTurntableSpeed()
{
  m_turntableRPM *= 2.0;
  displayMessage("Turntable " + speedToString());
}

void Viewer::decreaseTurntableSpeed()
{
  m_turntableRPM /= 2.0;
  displayMessage("Turntable " + speedToString());
}

void Viewer::updateSpin()
{

  manipulatedFrame()->rotateAroundPoint(Quaternion(m_turntableUp,
                                        (M_PI * 2.0 * m_turntableRPM)/(30.0 * 60)),
                                        camera()->frame()->revolveAroundPoint());
}

void Viewer::toggleLogo()
{
  m_showLogo = !m_showLogo;
  update();
}

void Viewer::savePathMovie()
{
  setSnapshotFormat("PNG");
  setSnapshotFileName("Snap");

  connect(this, SIGNAL(drawFinished(bool)), this, SLOT(saveSnapshot(bool)));

  // Loop over camera positions
  KeyFrameInterpolator *kfi = camera()->keyFrameInterpolator(1);

  for(int i = 0; i < kfi->numberOfKeyFrames(); ++i)
  {
    camera()->setFieldOfView(m_fov.at(i));
    kfi->interpolateAtTime(kfi->keyFrameTime(i));
  }
  disconnect(this, SIGNAL(drawFinished(bool)), this, SLOT(saveSnapshot(bool)));

}

QString Viewer::speedToString()
{
  QString result;
  if(m_turntableRPM < 1.0)
  {
    // Convert to fraction
    result = QString("1/%1x").arg((int)(1.0/m_turntableRPM));
  } else {
    result = QString("%1x").arg((int)m_turntableRPM);
  }

  return result;
}
