#include "PointGenerator.h"
#include <QVector>
#include <QVector3D>
#include <QColor>

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

PointGenerator::PointGenerator(QObject *parent) :
  QObject(parent), m_cancel(false)
{
}

PointCloud PointGenerator::createPointCloud(QString shape, int count, bool asSurface)
{
  QVector<float> points;

  float delta = 0.001;

  if(shape == "Cube")
  {
    if(!asSurface)
      points = createPointVolume(count, Cube());
    else
      points = createPointSurface(count, Cube(1.0 - delta), delta);
  }
  if(shape == "Sphere")
  {
    if(!asSurface)
      points = createPointVolume(count, Sphere());
    else
      points = createPointSurface(count, Sphere(0.5 - delta), delta);
  }
  if(shape == "Torus")
  {
    if(!asSurface)
      points = createPointVolume(count, Torus());
    else
      points = createPointSurface(count, Torus(0.3 - delta, 0.2 - delta), delta);
  }
  if(shape == "Cylinder")
  {
    if(!asSurface)
      points = createPointVolume(count, Cylinder());
    else
      points = createPointSurface(count, Cylinder(0.5 - delta), delta);
  }

  // Return resulting cloud
  QVector<QVector3D> position;

  position.reserve(points.count()/3);

  for(int i = 0; i < points.count(); i += 3)
  {
    position.push_back(QVector3D(points[i + 0],
                                 points[i + 1],
                                 points[i + 2]));
  }

  return PointCloud(position);
}

template <typename Shape>
QVector<float> PointGenerator::createPointVolume(int count, Shape shape)
{
  QVector<float> result;
  double x, y, z;

  // Reset cancel state
  m_cancel = false;

  emit setRange(0, count);

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
      emit progress(i);
    }

    if(m_cancel)
      break;
  }

  emit progress(count);

  return result;
}

template <typename Shape>
QVector<float> PointGenerator::createPointSurface(int count, Shape shape, double delta)
{
  QVector<float> result;
  double x, y, z;

  // reset cancel state
  m_cancel = false;

  emit setRange(0, count);

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
      emit progress(i);
    }

    if(m_cancel)
      break;
  }

  emit progress(count);

  return result;
}

void PointGenerator::cancel()
{
  m_cancel = true;
}
