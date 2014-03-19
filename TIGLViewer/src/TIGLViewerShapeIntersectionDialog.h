#ifndef TIGLVIEWERSHAPEINTERSECTIONDIALOG_H
#define TIGLVIEWERSHAPEINTERSECTIONDIALOG_H

#include <QDialog>

#include "CTiglPoint.h"

namespace Ui {
class TIGLViewerShapeIntersectionDialog;
}

namespace tigl {
    class CTiglUIDManager;
}

class TIGLViewerShapeIntersectionDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TIGLViewerShapeIntersectionDialog(tigl::CTiglUIDManager&, QWidget *parent = 0);
    ~TIGLViewerShapeIntersectionDialog();

    // Returns 0 if shape/shape intersection is selected
    // Returns 1 if shape/plane intersection is selected
    int GetMode();
    
    // shape - shape
    QString GetShape1UID();
    QString GetShape2UID();
    
    // shape - plane
    QString GetShapeUID();
    tigl::CTiglPoint GetPoint();
    tigl::CTiglPoint GetNormal();

private slots:
    void OnItemChanged();

private:
    
    Ui::TIGLViewerShapeIntersectionDialog *ui;
    tigl::CTiglUIDManager& uidManager;
    
    static tigl::CTiglPoint p;
    static tigl::CTiglPoint n;
    static int shape1Selected;
    static int shape2Selected;
    static int shapeSelected;
    static int lastMode;
};

#endif // TIGLVIEWERSHAPEINTERSECTIONDIALOG_H
