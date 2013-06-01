#ifndef CREATEPOINTCLOUDDIALOG_H
#define CREATEPOINTCLOUDDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
    class CreatePointCloudDialog;
}

class CreatePointCloudDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreatePointCloudDialog(QWidget *parent = 0);
    ~CreatePointCloudDialog();

public slots:
  void apply();

signals:
  void accepted(QString type, int count, bool surface);

private:
    Ui::CreatePointCloudDialog *ui;
};

#endif // CREATEPOINTCLOUDDIALOG_H
