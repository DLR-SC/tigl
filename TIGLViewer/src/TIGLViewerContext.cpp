/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerContext.cpp 203 2012-09-25 08:47:55Z martinsiggel $
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
#include "tigl_config.h"


#include <QApplication>
#include <QTextStream>
#include <QFile>
#include <qnamespace.h>
#include "TIGLViewerContext.h"
#include "TIGLViewerInternal.h"
#include "TIGLViewerSettings.h"
#include "TIGLViewerUndoCommands.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"
#include "PNamedShape.h"

#include "ISession_Point.h"
#include "ISession_Text.h"
#include "ISession_Direction.h"
#include "AIS_TexturedShape.hxx"
#include "AIS_InteractiveContext.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"

#include <OpenGl_GraphicDriver.hxx>
// Shader related stuff
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,7,0)
  #include <Prs3d_ShadingAspect.hxx>
  #include <Graphic3d_AspectFillArea3d.hxx>
  #include <Graphic3d_ShaderObject.hxx>
#endif

/// Loads a shader file from the resource system
QString getShaderFile(const QString& filename)
{
    QString result;

    // First try to open shader file from current dir
    // If not available, load from resource
    // This makes it possible, to debug the shader code
    QFile inFile(filename);
    if (!inFile.open(QFile::ReadOnly | QFile::Text)) {
        inFile.setFileName(":/shaders/" + filename);
        if (!inFile.open(QFile::ReadOnly | QFile::Text)) {
            return "";
        }
    }
    else {
        LOG(WARNING) << "Loading shader file \"" << filename.toStdString() << "\" from working directory.";
    }

    QTextStream stream(&inFile);
    result = stream.readAll();

    return result;
}

TIGLViewerContext::TIGLViewerContext(QUndoStack* stack)
    : myUndoStack(stack)
{
    // Create the OCC Viewers
    TCollection_ExtendedString a3DName("Visual3D");
    myViewer = createViewer( a3DName.ToExtString(), "", 1000.0 );
    myViewer->SetDefaultLights();
    myViewer->SetDefaultViewProj( V3d_Zpos );    // Top view
    myContext = new AIS_InteractiveContext( myViewer );

    myGridType       = Aspect_GT_Rectangular;
    myGridMode       = Aspect_GDM_Lines;
    myGridColor      = Quantity_NOC_RED4;
    myGridTenthColor = Quantity_NOC_GRAY90;

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,2,0)
    Handle(Prs3d_Drawer) whiteStyle = new Prs3d_Drawer();
    whiteStyle->SetColor(Quantity_NOC_WHITE);
    myContext->SetHighlightStyle(whiteStyle);
#elif OCC_VERSION_HEX >= VERSION_HEX_CODE(7,1,0)
    Handle(Graphic3d_HighlightStyle) whiteStyle = new Graphic3d_HighlightStyle;
    whiteStyle->SetColor(Quantity_NOC_WHITE);
    myContext->SetHighlightStyle(whiteStyle);
#else
    myContext->SetHilightColor(Quantity_NOC_WHITE);
#endif
    myContext->SetIsoNumber(0);

    // load shader
    initShaders();

    setGridOffset (0.0);
    gridXY();
    gridOn();
}

void TIGLViewerContext::initShaders()
{
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,0,0)
    QString shaderVersion = "v7";
#elif OCC_VERSION_HEX >= VERSION_HEX_CODE(6,7,0)
    QString shaderVersion = "v6";
#endif

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,7,0)
    myShader = new Graphic3d_ShaderProgram();

    // Attach vertex shader
    myShader->AttachShader (
        Graphic3d_ShaderObject::CreateFromSource(
            Graphic3d_TOS_VERTEX,
            getShaderFile("PhongShading-" + shaderVersion + ".vs").toStdString().c_str()
    ));

    // Attach fragment shader
    myShader->AttachShader (
        Graphic3d_ShaderObject::CreateFromSource(
            Graphic3d_TOS_FRAGMENT,
            getShaderFile("PhongShading-" + shaderVersion + ".fs").toStdString().c_str()
    ));

    myShader->PushVariable ("enableZebra", 0);
#endif
}

TIGLViewerContext::~TIGLViewerContext()
{

}

Handle(V3d_Viewer)& TIGLViewerContext::getViewer()
{ 
    return myViewer; 
}

Handle(AIS_InteractiveContext)& TIGLViewerContext::getContext()
{ 
    return myContext; 
}

Handle(V3d_Viewer) TIGLViewerContext::createViewer( const Standard_ExtString aName,
                                                    const Standard_CString aDomain,
                                                    const Standard_Real ViewSize )
{
    static Handle(OpenGl_GraphicDriver) deviceHandle;

    if (deviceHandle.IsNull()) {
      Handle(Aspect_DisplayConnection) aDisplayConnection;
#if !defined(_WIN32) && !defined(__WIN32__) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX))
      aDisplayConnection = new Aspect_DisplayConnection (qgetenv ("DISPLAY").constData());
#endif
      deviceHandle = new OpenGl_GraphicDriver (aDisplayConnection);
    }

#if OCC_VERSION_HEX < VERSION_HEX_CODE(7,4,0)
    return new V3d_Viewer(  deviceHandle,
                            aName,
                            aDomain,
                            ViewSize,
                            V3d_XposYnegZpos,
                            Quantity_NOC_BLACK,
                            V3d_ZBUFFER,
                            V3d_GOURAUD
  #if OCC_VERSION_HEX < VERSION_HEX_CODE(7,2,0)
                            , V3d_WAIT
  #endif
                         );
#else
    return new V3d_Viewer( deviceHandle );
#endif
}

/*! 
\brief    Deletes all objects.

        This function deletes all dispayed objects from the AIS context.
        No parameters.
*/
void TIGLViewerContext::deleteAllObjects()
{
    AIS_ListOfInteractive aList;
    myContext->DisplayedObjects( aList );
    AIS_ListIteratorOfListOfInteractive aListIterator;
    for ( aListIterator.Initialize( aList ); aListIterator.More(); aListIterator.Next() ) {
        myContext->Remove( aListIterator.Value(), Standard_False);
    }
    myContext->UpdateCurrentViewer();
}
/*! 
\brief    Sets the privileged plane to the XY Axis.  
*/
void TIGLViewerContext::gridXY  ( void )
{
    myGridColor      = Quantity_NOC_RED4;
    myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
    gp_Ax3 aPlane(gp_Pnt( 0., 0., 0. ),gp_Dir(0., 0., 1.));
    myViewer->SetPrivilegedPlane( aPlane );
}
/*!
\brief    Sets the privileged plane to the XZ Axis.

          Note the negative direction of the Y axis.
          This is corrrect for a right-handed co-ordinate set.
*/
void TIGLViewerContext::gridXZ  ( void )
{
    myGridColor      = Quantity_NOC_BLUE4;
    myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
    gp_Ax3 aPlane( gp_Pnt(0., 0., 0.),gp_Dir(0., -1., 0.) );
    myViewer->SetPrivilegedPlane( aPlane );
}
/*! 
\brief    Sets the privileged plane to the XY Axis.
 */
void TIGLViewerContext::gridYZ  ( void )
{
    myGridColor      = Quantity_NOC_GREEN4;
    myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
    gp_Ax3 aPlane( gp_Pnt( 0., 0., 0.), gp_Dir( 1., 0., 0. ) );
    myViewer->SetPrivilegedPlane( aPlane );
}

/*!
 * \brief Toggles the grid on and off
 */
void TIGLViewerContext::toggleGrid(bool gridIsOn) 
{
    if (gridIsOn) {
        gridOn();
    }
    else {
        gridOff();
    }
}

/*!
\brief    Turn the grid on.
 */
void TIGLViewerContext::gridOn  ( void )
{
    myViewer->ActivateGrid( myGridType , myGridMode );
    myViewer->SetGridEcho ( Standard_True );
}

/*! 
\brief    Turn the grid off.
*/
void TIGLViewerContext::gridOff ( void )
{
    myViewer->DeactivateGrid();
    myViewer->SetGridEcho( Standard_False );
}

/*!
\brief    Select a rectangular grid
*/
void TIGLViewerContext::gridRect ( void )
{
    myGridType = Aspect_GT_Rectangular;
    myViewer->ActivateGrid( myGridType , myGridMode );
    myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
}
/*! 
\brief    Select a circular grid.
*/
void TIGLViewerContext::gridCirc ( void )
{
    myGridType = Aspect_GT_Circular;
    myViewer->ActivateGrid( myGridType , myGridMode );
    myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
}

/*!
 * \brief Toggles wireframe view.
 * \param wireframe True, if wireframes should be printed
 */
void TIGLViewerContext::wireFrame(bool wireframe) 
{
    if (!myContext.IsNull()) {
        myContext->SetDisplayMode(wireframe ? AIS_WireFrame : AIS_Shaded, true);
    }
}

void TIGLViewerContext::selectAll()
{
    if (!myContext.IsNull()) {
        AIS_ListOfInteractive aList;
        // deselect all
        myContext->ClearSelected(Standard_False);
        myContext->DisplayedObjects( aList );
        AIS_ListIteratorOfListOfInteractive aListIterator;
        for ( aListIterator.Initialize( aList ); aListIterator.More(); aListIterator.Next() ) {
            // add to selection
            myContext->AddOrRemoveSelected(aListIterator.Value(), Standard_False);
        }
        myContext->UpdateCurrentViewer();
    }
}

void TIGLViewerContext::setGridOffset (Standard_Real offset)
{
    Standard_Real radius;
    Standard_Real xSize, ySize;
    Standard_Real oldOffset;
    
    myViewer->CircularGridGraphicValues( radius, oldOffset );
    myViewer->SetCircularGridGraphicValues( radius, offset);

    myViewer->RectangularGridGraphicValues(xSize, ySize, oldOffset);
    myViewer->SetRectangularGridGraphicValues(xSize, ySize, offset);
}

// a small helper when we just want to display a shape
void TIGLViewerContext::displayShape(const TopoDS_Shape& loft, bool updateViewer, Quantity_Color color, double transparency)
{
    TIGLViewerSettings& settings = TIGLViewerSettings::Instance();
    Handle(AIS_TexturedShape) shape = new AIS_TexturedShape(loft);

    myContext->SetMaterial(shape, settings.defaultMaterial(), Standard_False);
    myContext->SetColor(shape, color, Standard_False);
    myContext->SetTransparency(shape, transparency, Standard_False);
    shape->SetOwnDeviationCoefficient(settings.tesselationAccuracy());

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,7,0)
    if (!myShader.IsNull()) {
        shape->Attributes()->ShadingAspect()->Aspect()->SetShaderProgram (myShader);
    }
#endif

    myContext->Display(shape, updateViewer);
    
    if (settings.enumerateFaces()) {
        TopTools_IndexedMapOfShape shapeMap;
        TopExp::MapShapes(loft, TopAbs_FACE, shapeMap);
        for (int i = 1; i <= shapeMap.Extent(); ++i) {
            const TopoDS_Face& face = TopoDS::Face(shapeMap(i));
            gp_Pnt p = GetCentralFacePoint(face);
            QString s = QString("%1").arg(i);
            displayPoint(p, s.toStdString().c_str(), false, 0., 0., 0., 10.);
        }
    }
}

// a small helper when we just want to display a shape
void TIGLViewerContext::displayShape(const PNamedShape& pshape, bool updateViewer, Quantity_Color color, double transparency)
{
    if (!pshape) {
        return;
    }

    TIGLViewerSettings& settings = TIGLViewerSettings::Instance();
    Handle(AIS_TexturedShape) shape = new AIS_TexturedShape(pshape->Shape());

    myContext->SetMaterial(shape, settings.defaultMaterial(), Standard_False);
    myContext->SetColor(shape, color, Standard_False);
    myContext->SetTransparency(shape, transparency, Standard_False);
    shape->SetOwnDeviationCoefficient(settings.tesselationAccuracy());

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,7,0)
    if (!myShader.IsNull()) {
        shape->Attributes()->ShadingAspect()->Aspect()->SetShaderProgram (myShader);
    }
#endif

    QUndoCommand* command = new TiGLViewer::DrawObjects(myContext, shape, pshape->Name(), updateViewer);
    myUndoStack->push(command);

    if (settings.enumerateFaces()) {
        TopTools_IndexedMapOfShape shapeMap;
        TopExp::MapShapes(pshape->Shape(), TopAbs_FACE, shapeMap);
        for (int i = 1; i <= shapeMap.Extent(); ++i) {
            const TopoDS_Face& face = TopoDS::Face(shapeMap(i));
            std::string faceName = pshape->GetFaceTraits(i - 1).Name();
            if (faceName != std::string(pshape->Name())) {
                faceName = faceName + " (" + std::string(pshape->Name()) + ")";
            }
            gp_Pnt p = GetCentralFacePoint(face);
            QString s = QString("%1 - %2").arg(i).arg(faceName.c_str());
            displayPoint(p, s.toStdString().c_str(), false, 0., 0., 0., 10.);
        }
    }
    GetShapeManager().addObject(pshape, shape);
}

// Displays a point on the screen
void TIGLViewerContext::displayPoint(const gp_Pnt& aPoint,
                                     const char* aText,
                                     Standard_Boolean UpdateViewer,
                                     Standard_Real anXoffset,
                                     Standard_Real anYoffset,
                                     Standard_Real aZoffset,
                                     Standard_Real TextScale)
{
    if (std::string(aText).empty()) {
        displayShape(BRepBuilderAPI_MakeVertex(gp_Pnt(aPoint.X() + anXoffset, aPoint.Y() + anYoffset, aPoint.Z() + aZoffset)), UpdateViewer, Quantity_NOC_YELLOW);
    }
    else {
        Handle(ISession_Point) aGraphicPoint = new ISession_Point(aPoint.X(), aPoint.Y(), aPoint.Z());
        myContext->Display(aGraphicPoint,UpdateViewer);
        Handle(ISession_Text) aGraphicText = new ISession_Text(aText, aPoint.X() + anXoffset,
                                                     aPoint.Y() + anYoffset,
                                                     aPoint.Z() + aZoffset);
        aGraphicText->SetScale(TextScale);
        myContext->Display(aGraphicText,UpdateViewer);
    }

}

// convenience wrapper
void TIGLViewerContext::drawPoint(double x, double y, double z)
{
    displayPoint(gp_Pnt(x,y,z), "", Standard_True, 0,0,0, 1.0);
}

// Displays a vector on the screen
void TIGLViewerContext::displayVector(const gp_Pnt& aPoint,
                                      const gp_Vec& aVec,
                                      const char* aText,
                                      Standard_Boolean UpdateViewer,
                                      Standard_Real anXoffset,
                                      Standard_Real anYoffset,
                                      Standard_Real aZoffset,
                                      Standard_Real TextScale)
{
    Handle(ISession_Direction) aGraphicDirection = new ISession_Direction(aPoint, aVec);
    myContext->Display(aGraphicDirection,UpdateViewer);
    Handle(ISession_Text) aGraphicText = new ISession_Text(aText, aPoint.X() + anXoffset,
                                                 aPoint.Y() + anYoffset,
                                                 aPoint.Z() + aZoffset);
    aGraphicText->SetScale(TextScale);
    myContext->Display(aGraphicText,UpdateViewer);
}

bool TIGLViewerContext::hasSelectedShapes() const
{
    if (myContext.IsNull()) {
        return false;
    }

    bool hasSelectedShapes = false;
    for (myContext->InitSelected(); myContext->MoreSelected (); myContext->NextSelected ()) {
        if (myContext->IsDisplayed(myContext->SelectedInteractive())) {
            hasSelectedShapes = true;
        }
    }

    return hasSelectedShapes;
}

void TIGLViewerContext::updateViewer()
{
    if (!myContext.IsNull()) {
        myContext->UpdateCurrentViewer();
    }
}

// convenience wrapper
void TIGLViewerContext::drawVector(double x, double y, double z, double dirx, double diry, double dirz)
{
    displayVector(gp_Pnt(x,y,z), gp_Vec(dirx, diry, dirz), "", Standard_True, 0,0,0, 1.0);
}

std::vector<Handle(AIS_InteractiveObject)> TIGLViewerContext::selected()
{
    std::vector<Handle(AIS_InteractiveObject)> objects;
    if (!myContext.IsNull()) {
        for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected()) {
            objects.push_back(myContext->SelectedInteractive());
        }
    }
    return objects;
}

void TIGLViewerContext::eraseSelected()
{
    if (!myContext.IsNull()) {
        QUndoCommand* command = new TiGLViewer::DeleteObjects(myContext, selected());
        myUndoStack->push(command);
    }
}

void TIGLViewerContext::setTransparency(int tr)
{
    Standard_Real transparency = 0.1;
    if ( (tr < 0) || (tr > 100)) {
        return;
    }
    transparency = tr * 0.01;

    if (!myContext.IsNull()) {
        for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected()) {
            myContext->SetTransparency(myContext->SelectedInteractive(), transparency, Standard_True);
        }
    }
}

void TIGLViewerContext::setObjectsWireframe()
{
    if (!myContext.IsNull()) {
        for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected()) {
            myContext->SetDisplayMode(myContext->SelectedInteractive(), 0, true);
        }
    }
}

void TIGLViewerContext::setObjectsShading()
{
    if (!myContext.IsNull()) {
        for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected()) {
            myContext->SetDisplayMode(myContext->SelectedInteractive(), 1, true);
        }
    }
}

void TIGLViewerContext::setObjectsMaterial(Graphic3d_NameOfMaterial material)
{
    if (!myContext.IsNull()) {
        for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected()) {
             myContext->SetMaterial (myContext->SelectedInteractive(),  material, true);
        }
    }
}

void TIGLViewerContext::setObjectsTexture(const QString &filename)
{
    if (!myContext.IsNull()) {
        QApplication::setOverrideCursor( Qt::WaitCursor );
        QApplication::processEvents();
        for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected()) {
             Handle(AIS_TexturedShape) shape = Handle(AIS_TexturedShape)::DownCast(myContext->SelectedInteractive());
             if (!shape.IsNull()) {
                 shape->SetTextureFileName(filename.toStdString().c_str());
                 shape->SetTextureMapOn();
                 if (shape->DisplayMode() == 3) {
                     myContext->RecomputePrsOnly (shape, true);
                 }
                 else {
                     myContext->SetDisplayMode (shape, 3, Standard_False);
                     myContext->Display        (shape, Standard_True);
                 }
             }
        }
        QApplication::restoreOverrideCursor();
    }
}

void TIGLViewerContext::setReflectionlinesEnabled(bool enable)
{
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,7,0)
    int value = enable? 1 : 0;

    if (!myShader.IsNull()) {
        myShader->PushVariable("enableZebra", value);
    }
    myContext->UpdateCurrentViewer();
#else
    LOG(ERROR) << "Reflection lines are only available from OpenCASCADE 6.7.0 and newer!" << std::endl;
#endif
}

void TIGLViewerContext::setObjectsColor(const QColor& color)
{
    if (color.isValid() && !myContext.IsNull()) {
        QUndoCommand* command = new TiGLViewer::ChangeObjectsColor(myContext, selected(), color);
        myUndoStack->push(command);
    }
}

void TIGLViewerContext::setFaceBoundariesEnabled(bool enabled) {
    if (myContext && myContext->DefaultDrawer()) {
        myContext->DefaultDrawer()->SetFaceBoundaryDraw(enabled);
    }
}

InteractiveShapeManager& TIGLViewerContext::GetShapeManager()
{
    return myShapeManager;
}



