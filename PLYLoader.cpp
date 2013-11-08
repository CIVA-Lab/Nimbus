#include "PLYLoader.h"
#include <QVector3D>
#include <QColor>

#include <QDebug>

PLYLoader::PLYLoader(QObject *parent) :
  QObject(parent), m_ply(false), m_pointCount(0), m_cancelLoad(false)
{
}

PLYLoader::~PLYLoader()
{
  // Cleanup handle if valid
  if(m_ply)
  {
    ply_close(m_ply);
    m_ply = NULL;
  }
}

bool PLYLoader::canRead(const QString &path)
{
  // Early exit for invalid path
  if(path.isEmpty()) return false;

  // Attempt open
  p_ply ply = ply_open(path.toLocal8Bit().constData(), nullErrorCallback, 0,
                       NULL);
  if(!ply) return false;

  // Try reading header
  bool result = ply_read_header(ply);

  // Clean up ply handle
  ply_close(ply);

  return result;
}

bool PLYLoader::open(const QString &path)
{
  // Try to open; pass this as user data
  m_ply = ply_open(path.toLocal8Bit().constData(), nullErrorCallback, 0, NULL);
  if(!m_ply) return false;

  // Try to read header
  if(!ply_read_header(m_ply))
  {
    ply_close(m_ply);
    m_ply = NULL;
  }

  // Register vertex callbacks
  m_pointCount = ply_set_read_cb(m_ply, "vertex", "x", vertexCallback, this, 0);
  ply_set_read_cb(m_ply, "vertex", "y", vertexCallback, this, 0);
  ply_set_read_cb(m_ply, "vertex", "z", vertexCallback, this, 0);

  // Register color callbacks
  ply_set_read_cb(m_ply, "vertex", "red", colorCallback, this, 0);
  ply_set_read_cb(m_ply, "vertex", "green", colorCallback, this, 0);
  ply_set_read_cb(m_ply, "vertex", "blue", colorCallback, this, 0);

  // Ignore alpha for now
  int alphaCount = ply_set_read_cb(m_ply, "vertex", "alpha", NULL, NULL, 0);
  if(alphaCount > 0)
    qWarning() << "Ignoring vertex alpha";

  // Load cameras
  ply_set_read_cb(m_ply, "camera", "x", cameraPositionCallback, this, 0);
  ply_set_read_cb(m_ply, "camera", "y", cameraPositionCallback, this, 0);
  ply_set_read_cb(m_ply, "camera", "z", cameraPositionCallback, this, 0);

  ply_set_read_cb(m_ply, "camera", "ux", cameraUpCallback, this, 0);
  ply_set_read_cb(m_ply, "camera", "uy", cameraUpCallback, this, 0);
  ply_set_read_cb(m_ply, "camera", "uz", cameraUpCallback, this, 0);

  ply_set_read_cb(m_ply, "camera", "dx", cameraAimCallback, this, 0);
  ply_set_read_cb(m_ply, "camera", "dy", cameraAimCallback, this, 0);
  ply_set_read_cb(m_ply, "camera", "dz", cameraAimCallback, this, 0);

  ply_set_read_cb(m_ply, "camera", "arx", cameraAspectCallback, this, 0);
  ply_set_read_cb(m_ply, "camera", "ary", cameraAspectCallback, this, 0);
  ply_set_read_cb(m_ply, "camera", "arz", cameraAspectCallback, this, 0);

  // Open is only successful if there are vertices to read
  return m_pointCount > 0;
}

PointCloud PLYLoader::load()
{
  // Try reading all vertex data
  if(!ply_read(m_ply)) return PointCloud();

  // Done with library handle
  ply_close(m_ply);
  m_ply = NULL;

  // If load was canceled
  if(m_cancelLoad)
    return PointCloud();

  // Make sure correct number of components
  if(m_pointCount != m_points.count()/3)
  {
    // Handle error here
  }

  // Convert interleaved vertex components to vector
  QVector<QVector3D> vertices;
  for(int i = 0; i < m_points.count(); i += 3)
  {
    QVector3D vertex(m_points.at(i + 0),
              m_points.at(i + 1),
              m_points.at(i + 2));

    vertices.push_back(vertex);
  }

  // Convert colors
  QVector<QColor> colors;
  for(int i = 0; i < m_colors.count(); i += 3)
  {
    QColor c;
    c.setRgb(m_colors.at(i + 0),
             m_colors.at(i + 1),
             m_colors.at(i + 2));
    colors.push_back(c);
  }

  return PointCloud(vertices, colors);
}

void PLYLoader::nullErrorCallback(p_ply, const char *)
{

}

int PLYLoader::vertexCallback(p_ply_argument arg)
{
  // Get pointer to loader object
  PLYLoader *loader;
  ply_get_argument_user_data(arg, (void **)&loader, NULL);

  // See if load has been canceled
  if(loader->m_cancelLoad) return 0;

  // save vertex coordinate
  loader->m_points.push_back(ply_get_argument_value(arg));

  loader->emitProgress();

  // A return of 1 indicates keep loading
  return 1;
}

int PLYLoader::colorCallback(p_ply_argument arg)
{
  // Get pointer to loader object
  PLYLoader *loader;
  ply_get_argument_user_data(arg, (void **)&loader, NULL);

  // See if load has been canceled
  if(loader->m_cancelLoad) return 0;

  // Push vertex value onto list of coordinates
  loader->m_colors.push_back(ply_get_argument_value(arg));

//  qDebug() << "Got color" << ply_get_argument_value(arg);

  // Return 1 to indicate keep loading
  return 1;

}

int PLYLoader::cameraPositionCallback(p_ply_argument arg)
{
  // Get pointer to loader object
  PLYLoader *loader;
  ply_get_argument_user_data(arg, (void **)&loader, NULL);

  loader->m_cameraPositions.push_back(ply_get_argument_value(arg));

  // See if load has been canceled
  if(loader->m_cancelLoad) return 0;

  return 1;
}

int PLYLoader::cameraUpCallback(p_ply_argument arg)
{
  // Get pointer to loader object
  PLYLoader *loader;
  ply_get_argument_user_data(arg, (void **)&loader, NULL);

  loader->m_cameraUps.push_back(ply_get_argument_value(arg));

  // See if load has been canceled
  if(loader->m_cancelLoad) return 0;

  return 1;
}

int PLYLoader::cameraAimCallback(p_ply_argument arg)
{
  // Get pointer to loader object
  PLYLoader *loader;
  ply_get_argument_user_data(arg, (void **)&loader, NULL);

  loader->m_cameraAims.push_back(ply_get_argument_value(arg));

  // See if load has been canceled
  if(loader->m_cancelLoad) return 0;

  return 1;
}

int PLYLoader::cameraAspectCallback(p_ply_argument arg)
{
  // Get pointer to loader object
  PLYLoader *loader;
  ply_get_argument_user_data(arg, (void **)&loader, NULL);

  loader->m_cameraAspects.push_back(ply_get_argument_value(arg));

  // See if load has been canceled
  if(loader->m_cancelLoad) return 0;

  return 1;
}


void PLYLoader::emitProgress()
{
  int step = qMax(1.0, (m_pointCount * 3.0)/100.0);

  if(m_points.count() % step == 0)
  {
    int percent = 100.0 * m_points.count()/(m_pointCount * 3.0);
    emit progress(percent);
  }
}
