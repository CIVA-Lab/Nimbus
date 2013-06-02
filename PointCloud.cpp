#include "PointCloud.h"
#include <QDebug>

PointCloud::PointCloud() : m_needsExtents(false)
{
}

PointCloud::PointCloud(const QVector<QVector3D> &points,
                       const QVector<QColor> &colors) : m_points(points),
  m_colors(colors),
  m_needsExtents(true)
{
}

PointCloud::~PointCloud()
{
}

const QVector3D & PointCloud::point(int index) const
{
  return m_points.at(index);
}

void PointCloud::setPoint(int index, const QVector3D &point)
{
  m_points.replace(index, point);
  m_needsExtents = true;
}

int PointCloud::count() const
{
  return m_points.count();
}

bool PointCloud::hasColor() const
{
  return !m_colors.isEmpty();
}

const QVector3D & PointCloud::boundingBoxMinimum() const
{
  if(m_needsExtents) calculateExtents();

  return m_min;
}

const QVector3D & PointCloud::boundingBoxMaximum() const
{
  if(m_needsExtents) calculateExtents();

  return m_max;
}

const QVector3D & PointCloud::boundingBoxCenter() const
{
  if(m_needsExtents) calculateExtents();

  return m_center;
}

QVector<float> PointCloud::pointData() const
{
  QVector<float> interleaved;
  foreach(QVector3D point, m_points)
  {
    interleaved.push_back(point.x());
    interleaved.push_back(point.y());
    interleaved.push_back(point.z());
  }

  return interleaved;
}

QVector<unsigned char> PointCloud::colorData() const
{
  QVector<unsigned char> result;

  foreach(QColor c, m_colors)
  {
    result.push_back(c.red());
    result.push_back(c.green());
    result.push_back(c.blue());
  }

  return result;
}

void PointCloud::shuffle()
{
  for(int i = m_points.count() - 1; i > 0; i--)
  {
    // Get random index between 0 and i
    int j = (double)qrand()/RAND_MAX * i;

    QVector3D tmp = m_points.at(i);
    m_points.replace(i, m_points.at(j));
    m_points.replace(j, tmp);

    if(hasColor())
    {
      QColor tmpColor = m_colors.at(i);
      m_colors.replace(i, m_colors.at(j));
      m_colors.replace(j, tmpColor);
    }
  }
}

PointCloud PointCloud::shuffled() const
{
  PointCloud result(*this);

  result.shuffle();

  return result;
}

void PointCloud::calculateExtents() const
{
  // Initialize min and max
  m_min = m_points.first();
  m_max = m_points.first();

  // Find min and max of all points
  foreach(QVector3D point, m_points)
  {
    if(point.x() < m_min.x()) m_min.setX(point.x());
    else if(point.x() > m_max.x()) m_max.setX(point.x());

    if(point.y() < m_min.y()) m_min.setY(point.y());
    else if(point.y() > m_max.y()) m_max.setY(point.y());

    if(point.z() < m_min.z()) m_min.setZ(point.z());
    else if(point.z() > m_max.z()) m_max.setZ(point.z());
  }

  // Calculate Center
  m_center = (m_max - m_min)/2.0;

  // Mark extents as calculated
  m_needsExtents = false;
}
