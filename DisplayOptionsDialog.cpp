#include "DisplayOptionsDialog.h"
#include "ui_DisplayOptionsDialog.h"

DisplayOptionsDialog::DisplayOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DisplayOptionsDialog)
{
  ui->setupUi(this);

  // Signal to Signal connections;  cause changes of private UI elements to
  // trigger public signals.
  connect(ui->pointSizeSpinBox, SIGNAL(valueChanged(int)), this,
          SIGNAL(pointSizeChanged(int)));
  connect(ui->pointDensitySpinBox, SIGNAL(valueChanged(int)),
          this, SIGNAL(pointDensityChanged(int)));
  connect(ui->smoothPointCheckBox, SIGNAL(toggled(bool)),
          SIGNAL(pointSmoothChanged(bool)));
  connect(ui->colorCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(pointColorChanged(bool)));
  connect(ui->depthMaskCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(pointDepthChanged(bool)));
  connect(ui->multisampleCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(multiSampleChanged(bool)));
}

DisplayOptionsDialog::~DisplayOptionsDialog()
{
  delete ui;
}

void DisplayOptionsDialog::setPointSize(int pointSize)
{
  if(ui->pointSizeSpinBox->value() != pointSize)
  {
    ui->pointSizeSpinBox->setValue(pointSize);
  }

  if(ui->pointSizeSlider->value() != pointSize)
  {
    ui->pointSizeSlider->setValue(pointSize);
  }
}

void DisplayOptionsDialog::setPointDensity(int density)
{
  if(ui->pointDensitySpinBox->value() != density)
  {
    ui->pointDensitySpinBox->setValue(density);
  }
  if(ui->pointDensitySlider->value() != density)
  {
    ui->pointDensitySlider->setValue(density);
  }
}

void DisplayOptionsDialog::setSmoothPoints(bool smoothPoints)
{
  ui->smoothPointCheckBox->setChecked(smoothPoints);
}

void DisplayOptionsDialog::setPointColor(bool color)
{
  ui->colorCheckBox->setChecked(color);
}

void DisplayOptionsDialog::setDepthMask(bool mask)
{
  ui->depthMaskCheckBox->setChecked(mask);
}

void DisplayOptionsDialog::setMultisample(bool multisample)
{
  ui->multisampleCheckBox->setChecked(multisample);
}

void DisplayOptionsDialog::setMultisampleAvailable(bool available)
{
  ui->multisampleCheckBox->setEnabled(available);
}
