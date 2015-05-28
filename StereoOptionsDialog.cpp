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
  connect(ui->swapCheckBox, SIGNAL(toggled(bool)), this,
          SIGNAL(swapLeftRightChanged(bool)));

  connect(ui->stereoModeCombo, SIGNAL(currentIndexChanged(int)), this,
          SLOT(updateStereoMode(int)));
}

StereoOptionsDialog::~StereoOptionsDialog()
{
  delete ui;
}

void StereoOptionsDialog::addStereoMode(const QString &name,
                                        Viewer::StereoMode mode)
{
  ui->stereoModeCombo->addItem(name, mode);
}

void StereoOptionsDialog::setIODistance(double d)
{
  if(ui->ioDistanceSpinBox->value() != d)
    ui->ioDistanceSpinBox->setValue(d);
}

void StereoOptionsDialog::setScreenWidth(double d)
{
  if(ui->screenWidthSpinBox->value() != d)
    ui->screenWidthSpinBox->setValue(d);
}

void StereoOptionsDialog::setFocusDistance(double d)
{
  if(ui->screenWidthSpinBox->value() != d)
    ui->focusDistanceSpinBox->setValue(d);
}

void StereoOptionsDialog::setSwapLeftRight(bool checked)
{
  ui->swapCheckBox->setChecked(checked);
}

void StereoOptionsDialog::setStereoMode(Viewer::StereoMode mode)
{
  int index = ui->stereoModeCombo->findData(mode);

  if(ui->stereoModeCombo->currentIndex() != index)
  {
    ui->stereoModeCombo->setCurrentIndex(index);
    emit stereoModeChanged(mode);
  }
}

void StereoOptionsDialog::updateStereoMode(int index)
{
  int mode = ui->stereoModeCombo->itemData(index).toInt();

  emit stereoModeChanged((Viewer::StereoMode)mode);
}
