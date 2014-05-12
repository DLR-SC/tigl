#include "TIGLViewerScreenshotDialog.h"
#include "ui_TIGLViewerScreenshotDialog.h"

#include <QFileInfo>

TIGLViewerScreenshotDialog::TIGLViewerScreenshotDialog(QString filename, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TiglViewerScreenshotDialog)
{
    ui->setupUi(this);
    ui->widthBox->selectAll();
    if (QFileInfo(filename).suffix() != "jpg") {
        // hide quality parameter box
        ui->qualityBox->setVisible(false);
    }
}

TIGLViewerScreenshotDialog::~TIGLViewerScreenshotDialog()
{
    delete ui;
}

void TIGLViewerScreenshotDialog::setImageSize(int width, int height)
{
    ui->widthBox->setValue(width);
    ui->heightBox->setValue(height);
    ui->widthBox->selectAll();
}

void TIGLViewerScreenshotDialog::getImageSize(int& width, int& height) const
{
    width = ui->widthBox->value();
    height = ui->heightBox->value();
}

void TIGLViewerScreenshotDialog::setQualityValue(int quality)
{
    ui->qualityBox_2->setValue(quality);
}

int TIGLViewerScreenshotDialog::getQualityValue() const
{
    return ui->qualityBox_2->value();
}
