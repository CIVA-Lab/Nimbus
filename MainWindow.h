#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGLViewer/qglviewer.h>
#include <QIODevice>
#include "DisplayOptionsDialog.h"
#include "CreatePointCloudDialog.h"
#include "Viewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  template <typename Shape>
  QVector<float> GeneratePointVolume(int count, Shape shape);

  template <typename Shape>
  QVector<float> GeneratePointSurface(int count, Shape shape,
                                      double epsilon = 0.01);

  bool canRead(const QString& path);

public slots:
  void openFile();
  void openFile(const QString& path);

//  void loadSphereVolume(int pointCount);
  void createPointCloud(QString shape, int points, bool asSurface);

protected:
  void closeEvent(QCloseEvent *);
  void dragEnterEvent(QDragEnterEvent *);
  void dropEvent(QDropEvent *);

private:
  Ui::MainWindow *ui;

  Viewer* m_viewer;

  DisplayOptionsDialog* m_displayOptions;
  CreatePointCloudDialog* m_createOptions;
};

#endif // MAINWINDOW_H
