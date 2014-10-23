#ifndef STEREOOPTIONSDIALOG_H
#define STEREOOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class StereoOptionsDialog;
}

class StereoOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit StereoOptionsDialog(QWidget *parent = 0);
  ~StereoOptionsDialog();

signals:
  void IODistanceChanged(double);
  void screenWidthChanged(double);
  void focusDistanceChanged(double);

public slots:
  void setIODistance(double d);
  void setScreenWidth(double d);
  void setFocusDistance(double d);

private:
  Ui::StereoOptionsDialog *ui;
};

#endif // STEREOOPTIONSDIALOG_H