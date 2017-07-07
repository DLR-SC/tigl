/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-03-18 Martin Siggel <martin.siggel@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef TIGLVIEWERSHAPEINTERSECTIONDIALOG_H
#define TIGLVIEWERSHAPEINTERSECTIONDIALOG_H

#include <QDialog>

#include "CTiglPoint.h"

namespace Ui
{
    class TIGLViewerShapeIntersectionDialog;
}

namespace tigl
{
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

    QString GetShapeSUID();
    tigl::CTiglPoint GetPoint1();
    tigl::CTiglPoint GetPoint2();
    tigl::CTiglPoint GetW();

private slots:
    void OnItemChanged();

private:
    Ui::TIGLViewerShapeIntersectionDialog *ui;
    tigl::CTiglUIDManager& uidManager;

    static tigl::CTiglPoint p;
    static tigl::CTiglPoint n;
    static tigl::CTiglPoint p1;
    static tigl::CTiglPoint p2;
    static tigl::CTiglPoint w;
    static int shape1Selected;
    static int shape2Selected;
    static int shapeSelected;
    static int shapeSSelected;
    static int lastMode;
};

#endif // TIGLVIEWERSHAPEINTERSECTIONDIALOG_H
