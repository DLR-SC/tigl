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
#ifndef _TIGLAISTriangulation_HeaderFile
#define _TIGLAISTriangulation_HeaderFile

#include <Standard_DefineHandle.hxx>

#include <AIS_InteractiveObject.hxx>
#include <Handle_Poly_Triangulation.hxx>
#include <Handle_TColStd_HArray1OfInteger.hxx>

DEFINE_STANDARD_HANDLE(TIGLAISTriangulation,AIS_InteractiveObject)

//! Interactive object that draws data from  Poly_Triangulation, optionally with colors associated <br>
//!          with each triangulation vertex. For maximum efficiency colors are represented as 32-bit integers <br>
//!          instead of classic Quantity_Color values. <br>
//!          Interactive selection of triangles and vertices is not yet implemented. <br>
class TIGLAISTriangulation : public AIS_InteractiveObject 
{

public:

  //! Constructs the Triangulation display object <br>
  Standard_EXPORT   TIGLAISTriangulation(const Handle(Poly_Triangulation)& aTriangulation);
  
//! Set the color for each node. <br>
//! Each 32-bit color is Alpha << 24 + Blue << 16 + Green << 8 + Red <br>
//! Order of color components is essential for further usage by OpenGL <br>
  Standard_EXPORT     void SetColors(const Handle(TColStd_HArray1OfInteger)& aColor) ;
  
//! Get the color for each node. <br>
//! Each 32-bit color is Alpha << 24 + Blue << 16 + Green << 8 + Red <br>
  Standard_EXPORT     Handle_TColStd_HArray1OfInteger GetColors() const;
  
  Standard_EXPORT     void SetTriangulation(const Handle(Poly_Triangulation)& aTriangulation) ;
  //! Returns Poly_Triangulation . <br>
  Standard_EXPORT     Handle_Poly_Triangulation GetTriangulation() const;

  DEFINE_STANDARD_RTTI(TIGLAISTriangulation)

protected:




private: 

  
  Standard_EXPORT   virtual  void Compute(const Handle(PrsMgr_PresentationManager3d)& aPresentationManager,const Handle(Prs3d_Presentation)& aPresentation,const Standard_Integer aMode = 0) ;
  
  Standard_EXPORT   virtual  void ComputeSelection(const Handle(SelectMgr_Selection)& aSelection,const Standard_Integer aMode) ;
  //! Attenuates 32-bit color by a given attenuation factor (0...1): <br>
//! aColor = Alpha << 24 + Blue << 16 + Green << 8 + Red <br>
//! All color components are multiplied by aComponent, the result is then packed again as 32-bit integer. <br>
//! Color attenuation is applied to the vertex colors in order to have correct visual result <br>
//! after glColorMaterial(GL_AMBIENT_AND_DIFFUSE). Without it, colors look unnatural and flat. <br>
  Standard_EXPORT     Standard_Integer AttenuateColor(const Standard_Integer aColor,const Standard_Real aComponent) ;

Handle_Poly_Triangulation myTriangulation;
Handle_TColStd_HArray1OfInteger myColor;
Standard_Integer myFlagColor;
Standard_Integer myNbNodes;
Standard_Integer myNbTriangles;
Standard_Real minx, miny, minz, maxx, maxy, maxz;


};





// other Inline functions and methods (like "C++: function call" methods)


#endif
