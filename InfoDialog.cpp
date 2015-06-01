#include "InfoDialog.h"
#include "ui_InfoDialog.h"

#include <QFormLayout>
#include <QLabel>

InfoDialog::InfoDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::InfoDialog)
{
  ui->setupUi(this);
}

InfoDialog::~InfoDialog()
{
  delete ui;
}

void InfoDialog::setOpenGLInfo(const QStringList& info)
{
  clearGroupBox(ui->openGLBox);
  QFormLayout *layout = new QFormLayout();

  foreach(const QString i, info)
  {
    QStringList pairs = i.split(';');
    layout->addRow(pairs[0] + QString(":"), new QLabel(pairs[1]));
  }

  ui->openGLBox->setLayout(layout);
}

void InfoDialog::setPointCloudInfo(const QStringList &info)
{
  clearGroupBox(ui->pointCloudBox);

  QFormLayout *layout = new QFormLayout();

  foreach(const QString i, info)
  {
    QStringList pairs = i.split(';');
    layout->addRow(pairs[0] + QString(":"), new QLabel(pairs[1]));
  }

  ui->pointCloudBox->setLayout(layout);

}

void InfoDialog::clearGroupBox(QGroupBox *box)
{
  QLayout* layout = box->layout();
  if(layout)
  {
    while(!layout->isEmpty())
    {
      QLayoutItem* child = layout->takeAt(0);

      child->widget()->deleteLater();
      delete child;
    }
  }
  delete layout;
}
