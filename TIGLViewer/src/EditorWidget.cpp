#include "EditorWidget.h"
#include "ui_EditorWidget.h"

EditorWidget::EditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditorWidget)
{
    ui->setupUi(this);
}

EditorWidget::~EditorWidget()
{
    delete ui;
}

ModificatorWingWidget* EditorWidget::getWingWidget()
{
    return ui->wingModificator;
}

ModificatorFuselageWidget* EditorWidget::getFuselageWidget()
{
    return ui->fuselageModificator;
}

ModificatorTransformationWidget* EditorWidget::getTransformationWidget()
{

    return ui->transformationModificator;
}

QWidget* EditorWidget::getApplyWidget()
{
    return ui->applyWidget;
}

QWidget *EditorWidget::getNoInterfaceWidget() {
    return ui->noInterfaceWidget;
}
