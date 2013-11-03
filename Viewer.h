#ifndef VIEWER_H
#define VIEWER_H

#include <QGLViewer/qglviewer.h>
#include <QGLBuffer>
#include <QPixmap>
#include "PointCloud.h"

using namespace qglviewer;
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

  void fastInteractionChanged(bool);

  void error(QString);

public slots:
  void setPointSize(int pointSize);
  void setPointDensity(int density);
  void setSmoothPoints(bool smoothPoints);
  void toggleSmoothPoints();

  void setColorPoints(bool value);
  void setDepthMasking(bool value);
  void setMultisample(bool value);

  void setFastInteraction(bool value);

  void restoreView();

  void toggleTurntable();
  void resetTurntable();
  void increaseTurntableSpeed();
  void decreaseTurntableSpeed();

  void updateSpin();

  void toggleLogo();

protected:
  void init();
  void draw();
  void postDraw();
  void fastDraw();
  void keyPressEvent(QKeyEvent *);

  bool bindToVertexBuffer(const QVector<float> &vertices);
  bool loadColorsToBuffer(const QVector<unsigned char> &colors);
  bool loadColorsToBuffer(const QVector<float> &colors);

private:
  QString speedToString();

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
  bool m_fastInteraction;

  int m_fastInteractionMax;

  // Onscreen logo
  QPixmap m_logoPixmap;
  GLuint m_logoTextureId;
  bool m_showLogo;

  // Turntable speed
  double m_turntableRPM;
  Vec m_turntableUp;
  bool m_turntableStarted;
};

#endif // VIEWER_H
