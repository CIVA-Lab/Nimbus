#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QGroupBox>

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
  Q_OBJECT

public:
  explicit InfoDialog(QWidget *parent = 0);
  ~InfoDialog();

public slots:
  void setOpenGLInfo(const QStringList &info);
  void setPointCloudInfo(const QStringList &info);

private:
  void clearGroupBox(QGroupBox *box);

  Ui::InfoDialog *ui;
};

#endif // INFODIALOG_H
