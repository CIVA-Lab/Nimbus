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

  bool setPointCloud(const PointCloud& cloud);

  bool multisampleAvailable() const;

  bool stereoEnabled() const { return m_stereo; }

  bool supportsHardwareStereo() const;

  QVector<float> m_fov;

  enum StereoMode
  {
    Hardware,
    Red_Cyan,
    Red_Blue,
    Side_by_Side,
    Stacked
  };

  StereoMode stereoMode() const { return m_stereoMode; }

  QStringList openGLInfo();
  QStringList pointCloudInfo();

signals:
  void pointSizeChanged(int pointSize);
  void pointDensityChanged(int pointDensity);
  void smoothPointsChanged(bool smoothPoints);
  void pointColorChanged(bool);
  void depthMaskingChanged(bool);
  void multisampleChanged(bool);

  void fastInteractionChanged(bool);

  // Signals for changes in stereo parameters
  void IODistanceChanged(double);
  void focusDistanceChanged(double);
  void physicalScreenWidthChanged(double);
  void stereoModeChanged(StereoMode);
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

  void setIODistance(double distance);
  void setScreenWidth(double width);
  void setFocusDistance(double distance);
  void setSwapLeftRight(bool swap);

  void setStereo(bool stereo = true);
  void toggleStereo();
  void setStereoMode(StereoMode mode);

  void toggleTurntable();
  void resetTurntable();
  void increaseTurntableSpeed();
  void decreaseTurntableSpeed();

  void updateSpin();

  void toggleLogo();

  void savePathMovie();

protected:
  void init();
  void draw();
  void drawRedCyanStereo();
  void drawRedBlueStereo();
  void drawSideBySideStereo();
  void drawStackedStereo();
  void drawHardwareStereo();
  void postDraw();
  void fastDraw();
  void paintGL();
  void keyPressEvent(QKeyEvent *);

  bool bindToVertexBuffer(const QVector<float> &vertices);
  bool loadColorsToBuffer(const QVector<float> &colors);

  void notifyStereoParametersChanged();

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

  // Swap eyes for stereo
  bool m_swapLeftRight;
  // Stereo enabled
  bool m_stereo;
  // Stereo mode
  StereoMode m_stereoMode;

  // Onscreen logo
  QPixmap m_logoPixmap;
  GLuint m_logoTextureId;
  bool m_showLogo;

  // Turntable speed
  double m_turntableRPM;
  Vec m_turntableUp;
  bool m_turntableStarted;

  PointCloud m_pointCloud;
};

#endif // VIEWER_H
