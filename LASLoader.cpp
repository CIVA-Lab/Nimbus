#include "LASLoader.h"
#include <QVector>
#include <QVector3D>
#include <QColor>
#include <QFile>
#include <QDebug>

#include <limits>

LASLoader::LASLoader(QObject *parent) : QObject(parent), m_pointCount(0),
  m_reader(NULL), m_cancel(false)
{
}

LASLoader::~LASLoader()
{
}

bool LASLoader::canRead(const QString &path)
{
  QFile lasFile(path);
  if(!lasFile.open(QIODevice::ReadOnly))
  {
    return false;
  }

  // Check magic number; laslib will attempt to read invalid files
  if(lasFile.read(4) != "LASF")
    return false;

  LASreadOpener opener;

  opener.set_file_name(path.toLocal8Bit().constData());
  LASreader* reader = opener.open();

  bool result = (reader != NULL);

  delete reader;

  return result;
}

bool LASLoader::open(const QString &path)
{
  LASreadOpener opener;

  opener.set_file_name(path.toLocal8Bit().constData());
  m_reader = opener.open();

  if(!m_reader) return false;

  m_pointCount = m_reader->npoints;

  return true;
}

PointCloud LASLoader::load()
{
  if(!m_reader) return PointCloud();

  QVector<QVector3D> points;
  QVector<QColor> color;

  int step = qMax(1.0, m_pointCount/100.0);

  while(m_reader->read_point())
  {
    points.push_back(QVector3D(m_reader->point.x,
                                 m_reader->point.y,
                                 m_reader->point.z));
    if(m_reader->point.have_rgb)
    {
      quint16 red = m_reader->point.rgb[0];
      quint16 green = m_reader->point.rgb[1];
      quint16 blue = m_reader->point.rgb[2];

      QColor c;

      quint16 max = std::numeric_limits<quint16>::max();

      c.setRedF((qreal)red/max);
      c.setGreenF((qreal)green/max);
      c.setBlueF((qreal)blue/max);

      color.push_back(c);
    }

    if(m_cancel)
      return PointCloud();

    if(points.count() % step == 0)
    {
      emit progress(100.0 * points.count()/m_pointCount);
    }
  }

  return PointCloud(points, color);
}

void LASLoader::cancel()
{
  m_cancel = true;
}

