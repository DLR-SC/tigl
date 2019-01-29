#include "CPACSTreeWidget.h"
#include "ui_CPACSTreeWidget.h"

CPACSTreeWidget::CPACSTreeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CPACSTreeWidget)
{
    ui->setupUi(this);
}

CPACSTreeWidget::~CPACSTreeWidget()
{
    delete ui;
}


QTreeView* CPACSTreeWidget::getQTreeView()
{
    return ui->treeView;
}