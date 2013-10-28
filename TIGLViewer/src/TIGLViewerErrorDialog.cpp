#include "TIGLViewerErrorDialog.h"
#include "ui_TIGLViewerErrorDialog.h"

TIGLViewerErrorDialog::TIGLViewerErrorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLViewerErrorDialog)
{
    ui->setupUi(this);
}

TIGLViewerErrorDialog::~TIGLViewerErrorDialog()
{
    delete ui;
}
