#include "TIGLViewerSelectWingAndFlapStatusDialog.h"
#include "ui_TIGLViewerSelectWingAndFlapStatusDialog.h"

TIGLViewerSelectWingAndFlapStatusDialog::TIGLViewerSelectWingAndFlapStatusDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLViewerSelectWingAndFlapStatusDialog)
{
    ui->setupUi(this);
}

TIGLViewerSelectWingAndFlapStatusDialog::~TIGLViewerSelectWingAndFlapStatusDialog()
{
    delete ui;
}

void TIGLViewerSelectWingAndFlapStatusDialog::on_horizontalSlider_valueChanged(int value)
{
    ui->label_4->setText("Value: " + QString::number(ui->horizontalSlider->value()) + "%");
}
