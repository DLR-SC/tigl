#ifndef TIGLVIEWERSCREENSHOTDIALOG_H
#define TIGLVIEWERSCREENSHOTDIALOG_H

#include <QDialog>

namespace Ui {
class TiglViewerScreenshotDialog;
}

class TIGLViewerScreenshotDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TIGLViewerScreenshotDialog(QString filename, QWidget *parent = 0);

    void setImageSize(int width, int height);
    void getImageSize(int&width, int& height) const;
    
    void setQualityValue(int quality);
    int  getQualityValue() const;

    ~TIGLViewerScreenshotDialog();
    
private:
    Ui::TiglViewerScreenshotDialog *ui;
};

#endif // TIGLVIEWERSCREENSHOTDIALOG_H
