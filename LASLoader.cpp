#include "LASLoader.h"
#include <liblas/liblas.hpp>
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
  bool result = true;

  try {
  // Open file stream
  std::ifstream ifs;
  ifs.open(path.toLocal8Bit().constData(), std::ios::in | std::ios::binary);

  // Get reader from factory
  liblas::ReaderFactory f;
  liblas::Reader reader = f.CreateWithStream(ifs);

  // Get header
  liblas::Header const& header = reader.GetHeader();

  result = (header.GetFileSignature() == "LASF");

  } catch (std::exception){
    result = false;
  }

  return result;
}

bool LASLoader::open(const QString &path)
{
  bool result = true;

  try {
  // Open file stream
  std::ifstream ifs;
  ifs.open(path.toLocal8Bit().constData(), std::ios::in | std::ios::binary);

  // Get reader from factory
  liblas::ReaderFactory f;
  liblas::Reader reader = f.CreateWithStream(ifs);

  // Get header
  liblas::Header const& header = reader.GetHeader();

  result = (header.GetFileSignature() == "LASF");

  m_pointCount = header.GetPointRecordsCount();

  } catch (std::exception){
    result = false;
  }

  m_path = path;

  return result;
}

PointCloud LASLoader::load()
{
  try {
  // Open file stream
  std::ifstream ifs;
  ifs.open(m_path.toLocal8Bit().constData(), std::ios::in | std::ios::binary);

  // Get reader from factory
  liblas::ReaderFactory f;
  liblas::Reader reader = f.CreateWithStream(ifs);

  // Get header
  liblas::Header const& header = reader.GetHeader();

  m_pointCount = header.GetPointRecordsCount();


  liblas::Schema const& schema = header.GetSchema();
  bool hasColor = schema.GetDimension("Red")
      || schema.GetDimension("Green")
      || schema.GetDimension("Blue");

  int step = qMax(1.0, m_pointCount/100.0);

  QVector<QVector3D> points;
  QVector<QColor> colors;

  while(reader.ReadNextPoint())
  {
    liblas::Point const& p = reader.GetPoint();

    if(hasColor)
    {
      liblas::Color const& c = p.GetColor();
      QColor color;
      color.setRedF(c.GetRed()/65535.0);
      color.setGreenF(c.GetGreen()/65535.0);
      color.setBlueF(c.GetBlue()/65535.0);
      colors.push_back(color);
    }
    points.push_back(QVector3D(p.GetX(), p.GetY(), p.GetZ()));

    if(m_cancel)
      return PointCloud();

    if(points.count() % step == 0)
    {
      emit progress(100.0 * points.count()/m_pointCount);
    }
  }

  return PointCloud(points, colors);

  } catch (std::exception){
    qDebug() << "Got an exception!";
  }

  return PointCloud();
}

void LASLoader::cancel()
{
  m_cancel = true;
}
