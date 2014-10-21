#include "StereoOptionsDialog.h"
#include "ui_StereoOptionsDialog.h"

StereoOptionsDialog::StereoOptionsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::StereoOptionsDialog)
{
  ui->setupUi(this);

  // Cause changes to dialog input to emit signals
  connect(ui->ioDistanceSpinBox, SIGNAL(valueChanged(double)), this,
          SIGNAL(IODistanceChanged(double)));
  connect(ui->screenWidthSpinBox, SIGNAL(valueChanged(double)), this,
          SIGNAL(screenWidthChanged(double)));
  connect(ui->focusDistanceSpinBox, SIGNAL(valueChanged(double)), this,
          SIGNAL(focusDistanceChanged(double)));
}

StereoOptionsDialog::~StereoOptionsDialog()
{
  delete ui;
}

void StereoOptionsDialog::setIODistance(double d)
{
  ui->ioDistanceSpinBox->setValue(d);
}

void StereoOptionsDialog::setScreenWidth(double d)
{
  ui->screenWidthSpinBox->setValue(d);
}

void StereoOptionsDialog::setFocusDistance(double d)
{
  ui->focusDistanceSpinBox->setValue(d);
}
