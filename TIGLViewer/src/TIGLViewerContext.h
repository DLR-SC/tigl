/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerContext.h 203 2012-09-25 08:47:55Z martinsiggel $
*
* Version: $Revision: 203 $
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
#ifndef TIGLVIEWERCONTEXT_H
#define TIGLVIEWERCONTEXT_H

#include "tigl_internal.h"

#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <V3d_Viewer.hxx>
#include <QObject>
#include "TIGLViewer.h"
#include "TIGLViewerColors.h"
#include <QMetaType>
#include <QUndoStack>
#include <Standard_Version.hxx>
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,7,0)
  #include <Graphic3d_ShaderProgram.hxx>
#endif
#include "PNamedShape.h"

class TopoDS_Shape;
class gp_Pnt;
class gp_Vec;

class QOCC_DECLSPEC TIGLViewerContext : public QObject
{

    Q_OBJECT

public:

    TIGLViewerContext(QUndoStack*);
    ~TIGLViewerContext() OVERRIDE;

    Handle(V3d_Viewer)&              getViewer();
    Handle(AIS_InteractiveContext)&  getContext();

    Handle(V3d_Viewer) createViewer( const Standard_ExtString aName,
                                     const Standard_CString aDomain,
                                     const Standard_Real ViewSize );

    void setGridOffset (Quantity_Length offset);
    
    void displayPoint(const gp_Pnt& aPoint,
                      const char*   aText,
                      Standard_Boolean UpdateViewer,
                      Standard_Real anXoffset,
                      Standard_Real anYoffset,
                      Standard_Real aZoffset,
                      Standard_Real TextScale);
    
    void displayVector(const gp_Pnt& aPoint,
                       const gp_Vec& aVec,
                       const char* aText,
                       Standard_Boolean UpdateViewer,
                       Standard_Real anXoffset,
                       Standard_Real anYoffset,
                       Standard_Real aZoffset,
                       Standard_Real TextScale);

    bool hasSelectedShapes() const;

    void updateViewer();

    // Function used to highlight (HL) shape (used by ModificatorManager)

    // display the shape using highlighting settings and return the AIS_InteractiveObject
    Handle(AIS_InteractiveObject)   displayShapeHLMode( const TopoDS_Shape &loft,
                                                        bool updateViewer = Standard_True,
                                                        Quantity_Color color = Quantity_NOC_Highlight,
                                                        double transparency = 0.);
    // create a display a line using highlighting settings and return the AIS_InteractiveObject
    Handle(AIS_InteractiveObject)   displayLineHLMode(  double Ax,
                                                        double Ay,
                                                        double Az,
                                                        double Bx,
                                                        double By,
                                                        double Bz,
                                                        bool updateViewer = Standard_True,
                                                        Quantity_Color color = Quantity_NOC_Highlight,
                                                        double transparency = 0.);
    // remove the shape referenced by shape of the scene
    void removeShape( Handle(AIS_InteractiveObject) shape);

public slots:
    void displayShape(const PNamedShape& pshape, bool updateViewer, Quantity_Color color= Quantity_NOC_ShapeCol, double transparency=0.66);
    void displayShape(const TopoDS_Shape& loft, bool updateViewer, Quantity_Color color = Quantity_NOC_ShapeCol, double transparency=0.66);

    void drawPoint(double x, double y, double z);
    void drawVector(double x, double y, double z, double dirx, double diry, double dirz);
    void deleteAllObjects();
    void gridXY     ( void );
    void gridXZ     ( void );
    void gridYZ     ( void );
    void toggleGrid ( bool );
    void gridOn     ( void );
    void gridOff    ( void );
    void gridRect   ( void );
    void gridCirc   ( void );
    void wireFrame  ( bool );
    void selectAll();
    void eraseSelected();
    void setTransparency(int tr);
    void setObjectsWireframe();
    void setObjectsShading();
    void setObjectsMaterial(Graphic3d_NameOfMaterial material);
    void setObjectsTexture(const QString& filename);
    void setReflectionlinesEnabled(bool);
    void setObjectsColor(const QColor &color);

signals:

    void error (int errorCode, QString& errorDescription);

private:
    std::vector<Handle(AIS_InteractiveObject)> selected();

    Handle_V3d_Viewer               myViewer;
    Handle_AIS_InteractiveContext   myContext;
    Aspect_GridType                 myGridType;
    Aspect_GridDrawMode             myGridMode;
    Quantity_NameOfColor            myGridColor;
    Quantity_NameOfColor            myGridTenthColor;
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,7,0)
    Handle(Graphic3d_ShaderProgram) myShader;
#endif
    QUndoStack* myUndoStack;

    void initShaders();
};

Q_DECLARE_METATYPE(TIGLViewerContext*)

#endif // TIGLVIEWERCONTEXT_H
