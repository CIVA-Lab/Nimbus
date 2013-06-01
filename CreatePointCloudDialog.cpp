#include "CreatePointCloudDialog.h"
#include "ui_CreatePointCloudDialog.h"

CreatePointCloudDialog::CreatePointCloudDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreatePointCloudDialog)
{
    ui->setupUi(this);
    ui->pointCountSpinBox->setMaximum(RAND_MAX);
    ui->pointCountSpinBox->setValue(100000);
    ui->volumeRadioButton->setChecked(true);

    adjustSize();

    connect(this, SIGNAL(accepted()), SLOT(apply()));
}

void CreatePointCloudDialog::apply()
{
  // Gather values from boxes
  int pointCount = ui->pointCountSpinBox->value();
  QString shape = ui->shapeComboBox->currentText();
  bool surface = ui->surfaceRadioButton->isChecked();

  emit accepted(shape, pointCount, surface);
}

CreatePointCloudDialog::~CreatePointCloudDialog()
{
    delete ui;
}
