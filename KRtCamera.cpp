#include <QFile>
#include <QTextStream>
#include "KRtCamera.h"

// Overloaded stream operator to load matrix; static to limit scope to
// this file.
static QTextStream& operator>>(QTextStream& stream, QMatrix3x3 &matrix)
{
  for(int row = 0; row < 3; row++)
  {
    for(int column = 0; column < 3; column++)
    {
      stream >> matrix(row, column);
    }
  }
  return stream;
}

// Overloaded stream operator to load vector; static to limit scope to this
// file.
static QTextStream& operator>>(QTextStream& stream, QVector3D &vector)
{
  for(int i = 0; i < 3; ++i)
    stream >> vector[i];

  return stream;
}

KRtCamera::KRtCamera()
{
}

KRtCamera::KRtCamera(const QMatrix3x3 &intrinsic, const QMatrix3x3 &rotation,
               const QVector3D &translation)
{
  init(intrinsic, rotation, translation);
}

KRtCamera KRtCamera::load(const QString &path)
{
    // Open file path for text reading
    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
      return load(&file);

    // On failure return null camera
    return KRtCamera();
}

KRtCamera KRtCamera::load(QIODevice *device)
{
  // Ensure device is valid and opened for reading
  if(device && device->isOpen())
  {
    // Create stream to parse text from device
    QTextStream stream(device);

    // Read in 3x3 row-major intrinsic matrix k
    QMatrix3x3 intrinsic;
    stream >> intrinsic;

    // Read in 3x3 row-major rotation matrix R
    QMatrix3x3 rotation;
    stream >> rotation;

    // Read in 3D vector for translation T
    QVector3D translation;
    stream >> translation;

    if(stream.status() == QTextStream::Ok)
      return KRtCamera(intrinsic, rotation, translation);

  }
  // On failure return null camera
  return KRtCamera();
}

bool KRtCamera::isNull() const
{
  return m_complete.isIdentity();
}

QVector3D KRtCamera::position() const
{
  return m_position;
}

// Camera direction along positive z
QVector3D KRtCamera::direction() const
{
  // Direction is row 2 of rotation matrix
  return QVector3D(m_rotation(2, 0), m_rotation(2, 1), m_rotation(2, 2));
}

// Camera up is along negative y
QVector3D KRtCamera::up() const
{
  // Up is negative row 1 of rotation matrix
  return -QVector3D(m_rotation(1, 0), m_rotation(1, 1), m_rotation(1, 2));
}

void KRtCamera::lookAt(const QVector3D &eye, const QVector3D &at, const QVector3D &up)
{
  QVector3D zaxis = (at - eye).normalized();
  QVector3D xaxis = QVector3D::crossProduct(up, zaxis).normalized();
  QVector3D yaxis = QVector3D::crossProduct(zaxis, xaxis);

  QMatrix3x3 R;

  for(int i = 0; i < 3; ++i)
  {
    R(0, i) = xaxis[i];
    R(1, i) = yaxis[i];
    R(2, i) = zaxis[i];
  }

  QVector3D t(-QVector3D::dotProduct(xaxis, eye),
              -QVector3D::dotProduct(yaxis, eye),
              -QVector3D::dotProduct(zaxis, eye));

  init(m_intrinsic, R, t);
}

QSize KRtCamera::imagePlaneSize() const
{
  return QSize(m_intrinsic(0,2) * 2, m_intrinsic(1, 2) * 2);
}

QPointF KRtCamera::imagePlaneCenter() const
{
  return QPointF(m_intrinsic(0, 2), m_intrinsic(1, 2));
}

float KRtCamera::focalLength() const
{
  return m_intrinsic(0, 0);
}

QVector3D KRtCamera::directionThroughPixel(const QPointF &point) const
{
  return m_krInverse * QVector3D(point.x(), point.y() , 1.0f);
}

// Returns the 2D image plane coordinate of the 3D world position
QPointF KRtCamera::imageCoordinate(const QVector3D &world) const
{
  QVector3D p = m_complete.map(world);
  return QPointF(p.x()/p.z(), p.y()/p.z());
}

// Returns camera with intrinsics scaled by factor
KRtCamera KRtCamera::scaled(float factor) const
{
  QMatrix3x3 intrinsic = m_intrinsic * factor;
  intrinsic(2, 2) = 1;

  return KRtCamera(intrinsic, m_rotation, m_translation);
}

QString KRtCamera::toKRt() const
{
  QString result;
  QTextStream stream(&result);

  stream.setFieldAlignment(QTextStream::AlignRight);
  stream << qSetFieldWidth(12) << qSetRealNumberPrecision(6);

  for(int r = 0; r < 3; ++r)
  {
      stream << m_intrinsic(r, 0) << " "
             << m_intrinsic(r, 1) << " "
             << m_intrinsic(r, 2) << endl;
  }
  stream << endl;

  for(int r = 0; r < 3; ++r)
  {
    stream << m_rotation(r, 0) << " "
           << m_rotation(r, 1) << " "
           << m_rotation(r, 2) << endl;

  }
  stream << endl;

  stream << m_translation.x() << " "
         << m_translation.y() << " "
         << m_translation.z() << endl;

  return result;
}

void KRtCamera::init(const QMatrix3x3 &intrinsic, const QMatrix3x3 &rotation,
                  const QVector3D &translation)
{
  // Set intrinisc, rotation, and translation
  m_intrinsic = intrinsic;
  m_rotation = rotation;
  m_translation = translation;

  // Pre-compute inverse KR matrix; R^T * K^-1 for better precision
  m_krInverse = QMatrix4x4(m_rotation.transposed())
      * QMatrix4x4(m_intrinsic).inverted();

  // Augment rotation with translation
  QMatrix4x4 RT(rotation);
  // w needs to be 1.0 for augmented column
  RT.setColumn(3, QVector4D(m_translation, 1.0));

  // Create complete camera matrix C = K[R|T]
  m_complete = QMatrix4x4(m_intrinsic) * RT;

  // Convert camera extrinsics to camera position: C = -R^T * T
  m_position = QMatrix4x4(-m_rotation.transposed()) * m_translation;
}
