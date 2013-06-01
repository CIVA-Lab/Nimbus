#ifndef DISPLAYOPTIONSDIALOG_H
#define DISPLAYOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
    class DisplayOptionsDialog;
}

class DisplayOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DisplayOptionsDialog(QWidget *parent = 0);
    ~DisplayOptionsDialog();

signals:
  void pointSizeChanged(int pointSize);
  void pointDensityChanged(int density);
  void pointSmoothChanged(bool value);
  void pointColorChanged(bool value);
  void pointDepthChanged(bool value);
  void multiSampleChanged(bool value);

public slots:
  void setPointSize(int pointSize);
  void setPointDensity(int density);
  void setSmoothPoints(bool smoothPoints);
  void setPointColor(bool color);
  void setDepthMask(bool mask);
  void setMultisample(bool multisample);
  void setMultisampleAvailable(bool available);

private:
    Ui::DisplayOptionsDialog *ui;
};

#endif // DISPLAYOPTIONSDIALOG_H
