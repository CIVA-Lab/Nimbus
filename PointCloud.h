#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <QVector>
#include <QVector3D>
#include <QColor>

class PointCloud
{
public:
    PointCloud();
    PointCloud(const QVector<QVector3D> &points,
               const QVector<QColor> &colors = QVector<QColor>());

    ~PointCloud();

    const QVector3D& point(int index) const;
    void setPoint(int index, const QVector3D& point);
    int count() const;

    bool hasColor() const;

    const QVector3D& boundingBoxMinimum() const;
    const QVector3D& boundingBoxMaximum() const;
    const QVector3D& boundingBoxCenter() const;

    // Return x,y,z interleaved vector of floats
    QVector<float> pointData() const;
    // Return rgba interleaved values for color as unsigned bytes
    QVector<unsigned char> colorData() const;

    // Shuffle point order in-place
    void shuffle();
    // Return shuffled version of this point cloud
    PointCloud shuffled() const;

private:
    void calculateExtents() const;

    QVector<QVector3D> m_points;
    QVector<QColor> m_colors;

    // Following are mutable to allow logical constness
    mutable bool m_needsExtents;
    mutable QVector3D m_min;
    mutable QVector3D m_max;
    mutable QVector3D m_center;
};

#endif // POINTCLOUD_H
