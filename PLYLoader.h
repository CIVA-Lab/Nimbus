#ifndef PLYLOADER_H
#define PLYLOADER_H

#include <QObject>
#include <QVector>
#include "rply.h"
#include "PointCloud.h"

class PLYLoader : public QObject
{
  Q_OBJECT
public:
  explicit PLYLoader(QObject *parent = 0);
  ~PLYLoader();

  static bool canRead(const QString& path);

  bool open(const QString& path);
  int pointCount() const { return m_pointCount; }

  PointCloud load();

signals:
  void progress(int percent);

public slots:
  void cancel() { m_cancelLoad = true; }

private:
  static void nullErrorCallback(p_ply, const char *);
  static int vertexCallback(p_ply_argument arg);
  static int colorCallback(p_ply_argument arg);

  void emitProgress();

  p_ply m_ply;

  int m_pointCount;

  QVector<double> m_points;
  QVector<double> m_colors;

  bool m_cancelLoad;
};

#endif // PLYLOADER_H
