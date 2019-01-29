#ifndef CPACSTREEWIDGET_H
#define CPACSTREEWIDGET_H

#include <QWidget>
#include <QTreeView>

namespace Ui {
class CPACSTreeWidget;
}

class CPACSTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CPACSTreeWidget(QWidget *parent = nullptr);
    ~CPACSTreeWidget();

    QTreeView* getQTreeView();

private:
    Ui::CPACSTreeWidget *ui;
};

#endif // CPACSTREEWIDGET_H
