#ifndef LASLOADER_H
#define LASLOADER_H
#include <QObject>
#include <QString>
#include "lasreader.hpp"
#include "PointCloud.h"

class LASLoader : public QObject
{
  Q_OBJECT
public:
  explicit LASLoader(QObject *parent = 0);
  ~LASLoader();

  static bool canRead(const QString& path);

  bool open(const QString& path);
  int pointCount() const { return m_pointCount; }

  PointCloud load();

signals:
  void progress(int percent);

public slots:
  void cancel();

private:
  int m_pointCount;
  LASreader* m_reader;

  bool m_cancel;
};

#endif // LASLOADER_H
