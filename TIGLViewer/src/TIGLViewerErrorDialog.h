#ifndef TIGLVIEWERERRORDIALOG_H
#define TIGLVIEWERERRORDIALOG_H

#include <QDialog>

namespace Ui {
class TIGLViewerErrorDialog;
}

class TIGLViewerErrorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TIGLViewerErrorDialog(QWidget *parent = 0);
    
    void showMessage(const QString& msg);
    void setHistoryLog(ITiglLogger*);
    ~TIGLViewerErrorDialog();
    
private:
    Ui::TIGLViewerErrorDialog *ui;
};

#endif // TIGLVIEWERERRORDIALOG_H
