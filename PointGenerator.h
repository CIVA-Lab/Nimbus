#ifndef POINTGENERATOR_H
#define POINTGENERATOR_H

#include <QObject>
#include <QVector>
#include "PointCloud.h"

class PointGenerator : public QObject
{
  Q_OBJECT
public:
  explicit PointGenerator(QObject *parent = 0);
  PointCloud createPointCloud(QString shape, int count, bool asSurface);

  template <typename Shape>
  QVector<float> createPointVolume(int count, Shape shape);

  template <typename Shape>
  QVector<float> createPointSurface(int count, Shape shape, double delta);

signals:
  void progress(int);
  void setRange(int, int);
  
public slots:
  void cancel();

private:
  bool m_cancel;
};

#endif // POINTGENERATOR_H
