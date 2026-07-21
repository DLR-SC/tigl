/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLCreatorContext.h 203 2012-09-25 08:47:55Z martinsiggel $
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
#ifndef TIGLCREATORCONTEXT_H
#define TIGLCREATORCONTEXT_H

#include "tigl_internal.h"

#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <V3d_Viewer.hxx>
#include <QObject>
#include "TIGLCreator.h"
#include "TIGLCreatorColors.h"
#include "TIGLInteractiveShapeManager.h"
#include "TIGLCreatorSettings.h"
#include "DocumentId.h"
#include <QMetaType>
#include <QUndoStack>
#include <Standard_Version.hxx>
#include <map>
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,7,0)
  #include <Graphic3d_ShaderProgram.hxx>
#endif
#include "PNamedShape.h"

class TopoDS_Shape;
class gp_Pnt;
class gp_Vec;

class TIGLCreatorContext : public QObject
{

    Q_OBJECT

public:

    explicit TIGLCreatorContext(QUndoStack*);
    ~TIGLCreatorContext() override;

    Handle(V3d_Viewer)&              getViewer();
    Handle(AIS_InteractiveContext)&  getContext();
    Handle(V3d_Viewer) createViewer( const Standard_ExtString aName,
                                     const Standard_CString aDomain,
                                     const Standard_Real ViewSize );

    void setGridOffset (Standard_Real offset);
    
    Handle(AIS_Shape) displayPoint(const gp_Pnt& aPoint,
                      const char*   aText,
                      DocumentId docId,
                      Standard_Boolean UpdateViewer,
                      Standard_Real anXoffset,
                      Standard_Real anYoffset,
                      Standard_Real aZoffset,
                      Standard_Real TextScale);

    void displayVector(const gp_Pnt& aPoint,
                       const gp_Vec& aVec,
                       const char* aText,
                       DocumentId docId,
                       Standard_Boolean UpdateViewer,
                       Standard_Real anXoffset,
                       Standard_Real anYoffset,
                       Standard_Real aZoffset,
                       Standard_Real TextScale);

    bool hasSelectedShapes() const;

    void updateViewer();

    // Returns the shape manager tracking the named/UID shapes belonging to
    // one document. Created on first access.
    InteractiveShapeManager& GetShapeManager(DocumentId docId);

    // Cross-document lookup: resolves a picked AIS object back to its named
    // shape, regardless of which open document owns it. Used for 3D-view
    // picking, where the shared viewport can show shapes from any open
    // document.
    PNamedShape GetShapeFromIObject(const Handle(AIS_Shape)& obj);

    // Function used to highlight (HL) shape (used by ModificatorManager)

    // display the shape using highlighting settings and return the AIS_InteractiveObject
    Handle(AIS_InteractiveObject)   displayShapeHLMode( const TopoDS_Shape &loft,
                                                        DocumentId docId,
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
                                                        DocumentId docId,
                                                        bool updateViewer = Standard_True,
                                                        Quantity_Color color = Quantity_NOC_Highlight,
                                                        double transparency = 0.);
    // remove the shape referenced by shape of the scene
    void removeShape( Handle(AIS_InteractiveObject) shape);

    // Removes all objects (named and unnamed) belonging to one document from
    // the scene, without touching shapes owned by other open documents.
    void deleteObjectsOfDocument(DocumentId docId);

    // Registers an AIS object that was displayed outside of the
    // displayShape()/displayPoint()/... helpers (e.g. imported geometry
    // shown directly via getContext()->Display()) as belonging to a
    // document, so it participates in deleteObjectsOfDocument() scoping.
    void trackDisplayedObject(DocumentId docId, const Handle(AIS_InteractiveObject)& obj);

public slots:
    Handle(AIS_Shape) displayShape(const PNamedShape& pshape, DocumentId docId, bool updateViewer, Quantity_Color color= Quantity_NOC_ShapeCol, double transparency=0., bool shaded = true);
    Handle(AIS_Shape) displayShape(const TopoDS_Shape& loft, DocumentId docId, bool updateViewer, Quantity_Color color = Quantity_NOC_ShapeCol, double transparency=0., bool shaded = true);

    void drawPoint(double x, double y, double z, DocumentId docId = InvalidDocumentId);
    void drawVector(double x, double y, double z, double dirx, double diry, double dirz, DocumentId docId = InvalidDocumentId);
    // Removes all objects from the scene, across all open documents. Used
    // when shutting down the scene entirely (e.g. no documents left open).
    void deleteAllObjects();
    void gridXY     ();
    void gridXZ     ();
    void gridYZ     ();
    void toggleGrid ( bool );
    void gridOn     ();
    void gridOff    ();
    void gridRect   ();
    void gridCirc   ();
    void applyGridSettings ();
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
    void setObjectsColorRGB(int r, int g, int b, int a = 255);
    void setFaceBoundariesEnabled(bool enabled);
    void selectShape(const QString& uid);

signals:

    void error (int errorCode, QString& errorDescription);
    void displayAttributesChanged();
    void gridPlaneChanged(TIGLCreatorSettings::GridPlane plane);
    void shapeSelected(const QString& uid);

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

    // Named/UID shape lookup, scoped per document. Entries are created
    // lazily on first GetShapeManager(docId) access.
    std::map<DocumentId, InteractiveShapeManager> myShapeManagers;
    // Every AIS object displayed for a given document (named or not), used
    // to scope deleteObjectsOfDocument() without affecting other open
    // documents' shapes in the shared scene.
    std::map<DocumentId, std::vector<Handle(AIS_InteractiveObject)>> myDocumentObjects;

    void trackDocumentObject(DocumentId docId, const Handle(AIS_InteractiveObject)& obj);

    void initShaders();
};

Q_DECLARE_METATYPE(TIGLCreatorContext*)

#endif // TIGLCREATORCONTEXT_H
