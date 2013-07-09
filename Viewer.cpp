#include "Viewer.h"
#include <QDebug>
#include <QVector3D>
#include <QtCore/qmath.h>
#include <QAction>
#include <QGLShaderProgram>
#include <QGLShader>
#include <QKeyEvent>

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
  m_fastInteractionMax(250000)
{
  setAutoFillBackground(false);
  setKeyDescription(Qt::Key_P, "Toggle Smooth Points");
  setKeyDescription(Qt::Key_R, "Restore default view");
  setShortcut(EXIT_VIEWER, 0);
}

bool Viewer::setPointModel(const QVector<float> &points)
{
  // Make sure vector is not empty
  if(points.isEmpty())
    return false;

  // Bind points to VBO
  if(!bindToVertexBuffer(points))
    return false;

  if(m_colorBuffer.isCreated())
    m_colorBuffer.destroy();

  // Find min/max extents
  float minX, maxX;
  float minY, maxY;
  float minZ, maxZ;

  minX = maxX = points.at(0);
  minY = maxY = points.at(1);
  minZ = maxZ = points.at(2);

  for(int i = 3; i < points.count(); i += 3)
  {
    float x = points.at(i + 0);
    if(x < minX)
      minX = x;
    else if(x > maxX)
      maxX = x;

    float y = points.at(i + 1);
    if(y < minY)
      minY = y;
    else if(y > maxY)
      maxY = y;

    float z = points.at(i + 2);
    if(z < minZ)
      minZ = z;
    else if(z > maxZ)
      maxZ = z;
  }

  // Set scene's bounding box
  setSceneBoundingBox(qglviewer::Vec(minX, minY, minZ),
                      qglviewer::Vec(maxX, maxY, maxZ));

  // Update camera
  showEntireScene();

  // Restore point density to 100%; will also trigger point count display
  setPointDensity(100);

  return true;
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

  return true;
}

bool Viewer::multisampleAvailable()
{
  return format().testOption(QGL::SampleBuffers);
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

}

void Viewer::draw()
{
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

  glDrawArrays(GL_POINTS, 0, m_vertexCount * m_density/100.0);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  glDepthMask(GL_TRUE);
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

bool Viewer::loadColorsToBuffer(const QVector<unsigned char> &colors)
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

  m_vertexBuffer.allocate(colors.constData(),
                          colors.count() * sizeof(unsigned char));

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

  m_vertexBuffer.allocate(colors.constData(),
                          colors.count() * sizeof(float));

  return glGetError() == GL_NO_ERROR;
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
  default:
    QGLViewer::keyPressEvent(e);
  }
}
