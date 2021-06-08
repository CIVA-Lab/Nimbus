#include "LASLoader.h"
#include "lasreader_las.hpp"
#include <fstream>
#include <QVector>
#include <QVector3D>
#include <QColor>
#include <QDebug>

LASLoader::LASLoader(QObject *parent) :
  QObject(parent), m_cancel(false)
{
}

LASLoader::~LASLoader()
{

}

bool LASLoader::canRead(const QString &path)
{
  LASreaderLAS reader;

  return reader.open(qPrintable(path));
}

bool LASLoader::open(const QString &path)
{
  bool result = m_reader.open(qPrintable(path));

  m_pointCount = m_reader.npoints;

  return result;
}

PointCloud LASLoader::load()
{
  QVector<QVector3D> points;
  QVector<QColor> colors;

  points.reserve(m_pointCount);
  colors.reserve(m_pointCount);

  QVector<int> rawColors;
  rawColors.reserve(m_pointCount * 3);

  int step = qMax(1.0, m_pointCount/100.0);

  qDebug() << "Reading" << m_pointCount << "points.";
  while(m_reader.read_point())
  {
    points.push_back(QVector3D(m_reader.point.get_x(),
                               m_reader.point.get_y(),
                               m_reader.point.get_z()));

    QColor c;
    c.setRedF(m_reader.point.get_R()/65535.0f);
    c.setGreenF(m_reader.point.get_G()/65535.0f);
    c.setBlueF(m_reader.point.get_B()/65535.0f);

    colors.push_back(c);

//    if(m_reader.point.have_rgb)
//    {
//      rawColors.push_back(m_reader.point.get_R());
//      rawColors.push_back(m_reader.point.get_G());
//      rawColors.push_back(m_reader.point.get_B());
//    } else {
//      rawColors.push_back(m_reader.point.get_I());
//      rawColors.push_back(m_reader.point.get_I());
//      rawColors.push_back(m_reader.point.get_I());
//    }

    if(m_cancel) return PointCloud();

    if(points.count() % step == 0)
    {
      emit progress(100.0 * points.count()/m_pointCount);
    }

  }

  qDebug() << "Finding max color";
  // Determine color precision
 float maxColor = 255.0;

  for(float c : rawColors)
  {
    if(c > maxColor) maxColor = c;

    if(c > 255)
    {
      qDebug() << "breaking for color" << c;
      maxColor = 65535;
      break;
    }
  }

//  qDebug() << "Normalizing colors with max color" << maxColor;
//  // Normalize colors from 8-bit or 16-bit to 16-bit (0..255)
//  for(int i = 0; i < rawColors.count(); i+= 3)
//  {
//    colors.push_back(QColor(rawColors[i + 0]/maxColor * 255.0f,
//                            rawColors[i + 1]/maxColor * 255.0f,
//                            rawColors[i + 2]/maxColor * 255.0f));
//  }

  qDebug() << "Returning resulting point cloud";

  return PointCloud(points, colors);
//  try {
//  // Open file stream
//  std::ifstream ifs;
//  ifs.open(m_path.toLocal8Bit().constData(), std::ios::in | std::ios::binary);

//  // Get reader from factory
//  liblas::ReaderFactory f;
//  liblas::Reader reader = f.CreateWithStream(ifs);

//  // Get header
//  liblas::Header const& header = reader.GetHeader();

//  m_pointCount = header.GetPointRecordsCount();


//  liblas::Schema const& schema = header.GetSchema();
//  bool hasColor = schema.GetDimension("Red")
//      || schema.GetDimension("Green")
//      || schema.GetDimension("Blue");

//  int step = qMax(1.0, m_pointCount/100.0);

//  QVector<QVector3D> points;
//  QVector<QColor> colors;

//  while(reader.ReadNextPoint())
//  {
//    liblas::Point const& p = reader.GetPoint();

//    if(hasColor)
//    {
//      liblas::Color const& c = p.GetColor();
//      QColor color;
//      color.setRedF(c.GetRed()/65535.0);
//      color.setGreenF(c.GetGreen()/65535.0);
//      color.setBlueF(c.GetBlue()/65535.0);
//      colors.push_back(color);
//    }
//    points.push_back(QVector3D(p.GetX(), p.GetY(), p.GetZ()));

//    if(m_cancel)
//      return PointCloud();

//    if(points.count() % step == 0)
//    {
//      emit progress(100.0 * points.count()/m_pointCount);
//    }
//  }

//  return PointCloud(points, colors);

//  } catch (std::exception){
//    qDebug() << "Got an exception!";
//  }

  return PointCloud();
}

void LASLoader::cancel()
{
  m_cancel = true;
}
