#ifndef KRTCAMERA_H
#define KRTCAMERA_H
#include <QIODevice>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QPointF>
#include <QSize>
#include <QVector3D>

class KRtCamera
{
public:
  KRtCamera();
  KRtCamera(const QMatrix3x3& intrinsic, const QMatrix3x3& rotation,
         const QVector3D& translation);

  static KRtCamera load(const QString& path);
  static KRtCamera load(QIODevice *device);

  bool isNull() const;

  QVector3D position() const;
  QVector3D direction() const;
  QVector3D up() const;

  void lookAt(const QVector3D& eye, const QVector3D& at, const QVector3D& up);

  QSize imagePlaneSize() const;
  QPointF imagePlaneCenter() const;
  float focalLength() const;

  QVector3D directionThroughPixel(const QPointF& point) const;

  // Map 3D world coordinate to image plane pixel
  QPointF imageCoordinate(const QVector3D& world) const;

  KRtCamera scaled(float factor) const;

  QString toKRt() const;

  const QMatrix4x4 completeMatrix() const { return m_complete; }

private:
  void init(const QMatrix3x3& intrinsic, const QMatrix3x3& rotation,
            const QVector3D& translation);

  // Camera intrinsic matrix K
  QMatrix3x3 m_intrinsic;

  // Camera rotation matrix R
  QMatrix3x3 m_rotation;

  // Translation vector T
  QVector3D  m_translation;

  // Camera position in world coordintates
  QVector3D  m_position;

  // Inverse intrinsic-rotation matrix
  QMatrix4x4 m_krInverse;

  // Complete camera calibration matrix
  QMatrix4x4 m_complete;
};

#endif // KRTCAMERA_H
