#ifndef STEREOOPTIONSDIALOG_H
#define STEREOOPTIONSDIALOG_H

#include <QDialog>
#include "Viewer.h"

namespace Ui {
class StereoOptionsDialog;
}

class StereoOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit StereoOptionsDialog(QWidget *parent = 0);
  ~StereoOptionsDialog();

  void addStereoMode(const QString& name, Viewer::StereoMode mode);

signals:
  void IODistanceChanged(double);
  void screenWidthChanged(double);
  void focusDistanceChanged(double);
  void swapLeftRightChanged(bool);
  void stereoModeChanged(Viewer::StereoMode);

public slots:
  void setIODistance(double d);
  void setScreenWidth(double d);
  void setFocusDistance(double d);
  void setSwapLeftRight(bool);\
  void setStereoMode(Viewer::StereoMode);

private slots:
  void updateStereoMode(int index);

private:
  Ui::StereoOptionsDialog *ui;
};

#endif // STEREOOPTIONSDIALOG_H
