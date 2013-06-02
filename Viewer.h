#ifndef VIEWER_H
#define VIEWER_H

#include <QGLViewer/qglviewer.h>
#include <QGLBuffer>
#include "PointCloud.h"

class Viewer : public QGLViewer
{
    Q_OBJECT
public:
  explicit Viewer(QWidget *parent = 0);

  bool setPointModel(const QVector<float> &points);
  bool setPointCloud(const PointCloud& cloud);

  bool multisampleAvailable();

signals:
  void pointSizeChanged(int pointSize);
  void pointDensityChanged(int pointDensity);
  void smoothPointsChanged(bool smoothPoints);
  void pointColorChanged(bool);
  void depthMaskingChanged(bool);
  void multisampleChanged(bool);

  void error(QString);

public slots:
  void setPointSize(int pointSize);
  void setPointDensity(int density);
  void setSmoothPoints(bool smoothPoints);
  void toggleSmoothPoints();

  void setColorPoints(bool value);
  void setDepthMasking(bool value);
  void setMultisample(bool value);

protected:
  void init();
  void draw();
  void keyPressEvent(QKeyEvent *);

  bool bindToVertexBuffer(const QVector<float> &vertices);
  bool loadColorsToBuffer(const QVector<unsigned char> &colors);

private:
  // Vertex buffer object for point cloud
  QGLBuffer m_vertexBuffer;
  QGLBuffer m_colorBuffer;

  // Total number of vertices for point cloud
  int m_vertexCount;

  // Point cloud display options
  float m_density;
  float m_pointSize;
  bool m_smoothPoints;
  bool m_colorPoints;
  bool m_depthMasking;
  bool m_multisample;
};

#endif // VIEWER_H
