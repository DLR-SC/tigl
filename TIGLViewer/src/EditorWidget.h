#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>
#include "ModificatorWingWidget.h"
#include "ModificatorFuselageWidget.h"
#include "ModificatorTransformationWidget.h"


namespace Ui {
class EditorWidget;
}

class EditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget *parent = nullptr);
    ~EditorWidget();

    // getter for modificators
    ModificatorWingWidget* getWingWidget();
    ModificatorFuselageWidget* getFuselageWidget();
    ModificatorTransformationWidget* getTransformationWidget();
    QWidget* getApplyWidget();
    QWidget* getNoInterfaceWidget();

private:
    Ui::EditorWidget *ui;
};

#endif // EDITORWIDGET_H
