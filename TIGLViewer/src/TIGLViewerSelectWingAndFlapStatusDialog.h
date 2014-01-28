#ifndef TIGLVIEWERSELECTWINGANDFLAPSTATUSDIALOG_H
#define TIGLVIEWERSELECTWINGANDFLAPSTATUSDIALOG_H

#include <QDialog>

namespace Ui {
class TIGLViewerSelectWingAndFlapStatusDialog;
}

class TIGLViewerSelectWingAndFlapStatusDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TIGLViewerSelectWingAndFlapStatusDialog(QWidget *parent = 0);
    ~TIGLViewerSelectWingAndFlapStatusDialog();

private slots:
    void on_horizontalSlider_valueChanged(int value);

private:
    Ui::TIGLViewerSelectWingAndFlapStatusDialog *ui;
};

#endif // TIGLVIEWERSELECTWINGANDFLAPSTATUSDIALOG_H
