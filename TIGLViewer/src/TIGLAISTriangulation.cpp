// Copyright (c) 1999-2012 OPEN CASCADE SAS
//
// The content of this file is subject to the Open CASCADE Technology Public
// License Version 6.5 (the "License"). You may not use the content of this file
// except in compliance with the License. Please obtain a copy of the License
// at http://www.opencascade.org and read it completely before using this file.
//
// The Initial Developer of the Original Code is Open CASCADE S.A.S., having its
// main offices at: 1, place des Freres Montgolfier, 78280 Guyancourt, France.
//
// The Original Code and all software distributed under the License is
// distributed on an "AS IS" basis, without warranty of any kind, and the
// Initial Developer hereby disclaims all such warranties, including without
// limitation, any warranties of merchantability, fitness for a particular
// purpose or non-infringement. Please see the License for the specific terms
// and conditions governing the rights and limitations under the License.


#include <TIGLAISTriangulation.h>
#include <AIS_InteractiveObject.hxx>
#include <Standard_DefineHandle.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <Poly_Triangulation.hxx>
#include <Prs3d_Root.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <TShort_Array1OfShortReal.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_HArray1OfInteger.hxx>
#include <TShort_HArray1OfShortReal.hxx>
#include <Graphic3d_Group.hxx>
#include <Graphic3d_AspectFillArea3d.hxx>
#include <Graphic3d_ArrayOfTriangles.hxx>
#include <Select3D_SensitiveBox.hxx>
#include <Select3D_SensitiveTriangulation.hxx>
#include <SelectMgr_Selection.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Prs3d_Presentation.hxx>

IMPLEMENT_STANDARD_HANDLE(TIGLAISTriangulation, AIS_InteractiveObject)
IMPLEMENT_STANDARD_RTTIEXT(TIGLAISTriangulation, AIS_InteractiveObject)

namespace 
{
    inline double max(double a, double b) 
    {
        return a>b? a : b;
    }
    
    inline double min(double a, double b) 
    {
        return a<b? a : b;
    }
}

TIGLAISTriangulation::TIGLAISTriangulation(const Handle(Poly_Triangulation)& Triangulation)
{
    myTriangulation = Triangulation;
    myNbNodes       = Triangulation->NbNodes();
    myNbTriangles   = Triangulation->NbTriangles();
    myFlagColor     = 0;
    minx = DBL_MAX; 
    miny = DBL_MAX; 
    minz = DBL_MAX;
    maxx = DBL_MIN; 
    maxy = DBL_MIN; 
    maxz = DBL_MIN;
    
    const TColgp_Array1OfPnt& nodes = Triangulation->Nodes();
    for (int i = nodes.Lower(); i <= nodes.Upper(); ++i) {
        const gp_Pnt& p = nodes.Value(i);
        minx = min(minx, p.X());
        miny = min(miny, p.Y());
        minz = min(minz, p.Z());
        
        maxx = max(maxx, p.X());
        maxy = max(maxy, p.Y());
        maxz = max(maxz, p.Z());
    }
}

//=======================================================================
//function : Compute
//purpose  :
//=======================================================================
void TIGLAISTriangulation::Compute(const Handle(PrsMgr_PresentationManager3d)& aPresentationManager,
                                   const Handle(Prs3d_Presentation)& aPresentation,
                                   const Standard_Integer aMode)
{
    aPresentation->Clear();
    Handle(Graphic3d_Group) TheGroup = Prs3d_Root::CurrentGroup(aPresentation);
    TheGroup->Clear();

    switch (aMode) {
        case 1: {
            Handle(Graphic3d_AspectFillArea3d) aspect = myDrawer->ShadingAspect()->Aspect();
            Standard_Real ambient = aspect->FrontMaterial().Ambient();

            const TColgp_Array1OfPnt& nodes = myTriangulation->Nodes();             //Nodes
            const Poly_Array1OfTriangle& triangles = myTriangulation->Triangles();  //Triangle

            Standard_Boolean hasVNormals = myTriangulation->HasNormals();
            Standard_Boolean hasVColors  = (myFlagColor == 1);

            Handle(Graphic3d_ArrayOfTriangles) anArray =
                new Graphic3d_ArrayOfTriangles ( myNbNodes,        //maxVertexs
                        myNbTriangles * 3,//maxEdges
                        hasVNormals,      //hasVNormals
                        hasVColors,       //hasVColors
                        Standard_False    //hasTexels
                        );

            Standard_Integer i;
            Standard_Integer j;

            if (hasVNormals) {
                const TShort_Array1OfShortReal& normals = myTriangulation->Normals();
                if (hasVColors) {
                    const TColStd_Array1OfInteger& colors = myColor->Array1();
                    for ( i = nodes.Lower(); i <= nodes.Upper(); i++ ) {
                        j = (i - nodes.Lower()) * 3;
                        anArray->AddVertex(nodes(i), AttenuateColor(colors(i), ambient));
                        anArray->SetVertexNormal(i, normals(j+1), normals(j+2), normals(j+3));
                    }
                }
                // !hasVColors
                else {
                    for ( i = nodes.Lower(); i <= nodes.Upper(); i++ ) {
                        j = (i - nodes.Lower()) * 3;
                        anArray->AddVertex(nodes(i));
                        anArray->SetVertexNormal(i, normals(j+1), normals(j+2), normals(j+3));
                    }
                }
            }
            // !hasVNormals
            else {
                if (hasVColors) {
                    const TColStd_Array1OfInteger& colors = myColor->Array1();
                    for ( i = nodes.Lower(); i <= nodes.Upper(); i++ ) {
                        anArray->AddVertex(nodes(i), AttenuateColor(colors(i), ambient));
                    }
                }
                // !hasVColors
                else {
                    for ( i = nodes.Lower(); i <= nodes.Upper(); i++ ) {
                        anArray->AddVertex(nodes(i));
                    }
                }
            }

            Standard_Integer indexTriangle[3] = {0,0,0};
            for ( i = triangles.Lower(); i<= triangles.Upper(); i++ ) {
                triangles(i).Get(indexTriangle[0], indexTriangle[1], indexTriangle[2]);
                anArray->AddEdge(indexTriangle[0]);
                anArray->AddEdge(indexTriangle[1]);
                anArray->AddEdge(indexTriangle[2]);
            }
            TheGroup->SetPrimitivesAspect(aspect);
            TheGroup->AddPrimitiveArray(anArray);
            break;
        }
        case 0: {
            const TColgp_Array1OfPnt& nodes = myTriangulation->Nodes();
            const Poly_Array1OfTriangle& triangles = myTriangulation->Triangles();
            Handle_Graphic3d_AspectLine3d aspect = myDrawer->WireAspect()->Aspect();
     
            Handle(Graphic3d_ArrayOfPrimitives) segments =  new Graphic3d_ArrayOfSegments(nodes.Length(),triangles.Length()*6);
            for (Standard_Integer i = nodes.Lower(); i <= nodes.Upper(); i++ ) {
                segments->AddVertex(nodes(i));
            }
     
            Standard_Integer indexTriangle[3] = {0,0,0};
            for (Standard_Integer i = triangles.Lower(); i<= triangles.Upper(); i++ ) {
                triangles(i).Get(indexTriangle[0], indexTriangle[1], indexTriangle[2]);
                segments->AddEdge(indexTriangle[0]);
                segments->AddEdge(indexTriangle[1]);
                segments->AddEdge(indexTriangle[1]);
                segments->AddEdge(indexTriangle[2]);
                segments->AddEdge(indexTriangle[2]);
                segments->AddEdge(indexTriangle[0]);
            }
     
            TheGroup->SetPrimitivesAspect(aspect);
            TheGroup->AddPrimitiveArray(segments);
        }
    }
}

//=======================================================================
//function : ComputeSelection
//purpose  : 
//=======================================================================
void TIGLAISTriangulation::ComputeSelection(const Handle(SelectMgr_Selection)& aSelection,
                                            const Standard_Integer /*aMode*/)
{
    Handle(SelectMgr_EntityOwner) eown = new SelectMgr_EntityOwner(this,0);

    TopLoc_Location loc;
    Handle_Select3D_SensitiveTriangulation aSensitiveTria = new Select3D_SensitiveTriangulation(eown, myTriangulation, loc);
    aSelection->Add(aSensitiveTria);
}

//=======================================================================
//function : SetColor
//purpose  : Set the color for each node.
//           Each 32-bit color is Alpha << 24 + Blue << 16 + Green << 8 + Red
//           Order of color components is essential for further usage by OpenGL
//=======================================================================
void TIGLAISTriangulation::SetColors(const Handle(TColStd_HArray1OfInteger)& aColor)
{
    myFlagColor = 1;
    myColor = aColor;
}

//=======================================================================
//function : GetColor
//purpose  : Get the color for each node.
//           Each 32-bit color is Alpha << 24 + Blue << 16 + Green << 8 + Red
//           Order of color components is essential for further usage by OpenGL
//=======================================================================

Handle(TColStd_HArray1OfInteger) TIGLAISTriangulation::GetColors() const
{
    return myColor;
}


//=======================================================================
//function : SetTriangulation
//purpose  : 
//=======================================================================
void TIGLAISTriangulation::SetTriangulation(const Handle(Poly_Triangulation)& aTriangulation)
{
    myTriangulation = aTriangulation;
}

//=======================================================================
//function : GetTriangulation
//purpose  : 
//=======================================================================
Handle(Poly_Triangulation) TIGLAISTriangulation::GetTriangulation() const
{
    return myTriangulation;
}

//=======================================================================
//function : AttenuateColor
//purpose  : Attenuates 32-bit color by a given attenuation factor (0...1):
//           aColor = Alpha << 24 + Blue << 16 + Green << 8 + Red
//           All color components are multiplied by aComponent, the result is then packed again as 32-bit integer.
//           Color attenuation is applied to the vertex colors in order to have correct visual result 
//           after glColorMaterial(GL_AMBIENT_AND_DIFFUSE). Without it, colors look unnatural and flat.
//=======================================================================

Standard_Integer TIGLAISTriangulation::AttenuateColor( const Standard_Integer aColor,
                                                       const Standard_Real aComposition)
{
    Standard_Integer  red,
                      green,
                      blue,
                      alpha;

    alpha = aColor&0xff000000;
    alpha >>= 24;

    blue = aColor&0x00ff0000;
    blue >>= 16;

    green = aColor&0x0000ff00;
    green >>= 8;

    red = aColor&0x000000ff;
    red >>= 0; 

    red   = (Standard_Integer)(aComposition * red);
    green = (Standard_Integer)(aComposition * green);
    blue  = (Standard_Integer)(aComposition * blue);

    Standard_Integer  color;
    color = red;
    color += green << 8;
    color += blue  << 16; 
    color += alpha << 24;
    return color;
}

