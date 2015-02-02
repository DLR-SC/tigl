/* 
* This file was created by OpenCASCADE for the German Aerospace Center.
* It will reside in TiGL, until this Algorithm becomes part of
* OpenCASCADE.
* 
* For now, the file will be licensed under the Apache License, Version 2.0.
*    http://www.apache.org/licenses/LICENSE-2.0
*/

//! SurfTools class.
/*!
  This class provides static methods necessary for working with surfaces .
*/
//===================================================================
//
// Filename: SurfTools.hxx
//

#ifndef _SurfTools_HeaderFile
#define _SurfTools_HeaderFile

#ifndef _Handle_Geom_BSplineSurface_HeaderFile
#include <Handle_Geom_BSplineSurface.hxx>
#endif
#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif

class TColGeom_SequenceOfSurface;



#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SurfTools  {

public:

    void* operator new(size_t,void* anAddress) 
      {
        return anAddress;
      }
    void* operator new(size_t size) 
      { 
        return Standard::Allocate(size); 
      }
    void  operator delete(void *anAddress) 
      { 
        if (anAddress) Standard::Free((Standard_Address&)anAddress); 
      }
 // Methods PUBLIC
 // 

//! U reparametrization of B-Spline surface
/*! 
	\param theS surface
  \param theUfirst the value of the first parameter
  \param theUlast the value of the last parameter
  \return None
*/
Standard_EXPORT static  void UReparametrize(Handle_Geom_BSplineSurface& theS,
                const Standard_Real theUfirst,const Standard_Real theUlast) ;

//! V reparametrization of B-Spline surface
/*! 
	\param theS surface
  \param theVfirst the value of the first parameter
  \param theVlast the value of the last parameter
  \return None
*/
Standard_EXPORT static  void VReparametrize(Handle_Geom_BSplineSurface& theS,
                const Standard_Real theVfirst,const Standard_Real theVlast) ;

//! Smoothing B-Spline surface along U direction
/*! 
	\param theS surface
  \param theCont the value of continuity required
  \param theTol  the value of maximum allowed change of the shape of the surface 
  \param theInsertKnots the flag that allows to insert new knots. 
  \return None
*/
Standard_EXPORT static  void USmoothing(Handle_Geom_BSplineSurface& theS,
                const Standard_Integer theCont,const Standard_Real theTol,
                const Standard_Boolean theInsertKnots = Standard_True) ;

//! Smoothing B-Spline surface along V direction
/*! 
	\param theS surface
  \param theCont the value of continuity required 
  \param theTol  the value of maximum allowed change of the shape of the surface
  \param theInsertKnots the flag that allows to insert new knots. 
  \return None
*/
Standard_EXPORT static  void VSmoothing(Handle_Geom_BSplineSurface& theS,
                const Standard_Integer theCont,const Standard_Real theTol,
                const Standard_Boolean theInsertKnots = Standard_True) ;

//! Concatenate two surfaces along V boundary
/*! 
	\param theS1 the first surface
  \param theS1 the second surface
  \param theTol the value of sewing tolerance
  \param theS resulting surface 
  \return 0 if the concatenation has been done successfully
*/
Standard_EXPORT static  Standard_Integer ConcatenateSurfacesV(const Handle_Geom_BSplineSurface& theS1,
                const Handle_Geom_BSplineSurface& theS2,const Standard_Real theTol,Handle_Geom_BSplineSurface& theS) ;

//! Concatenate two surfaces along U boundary
/*! 
	\param theS1 the first surface
  \param theS1 the second surface
  \param theTol the value of sewing tolerance
  \param theS resulting surface 
  \return 0 if the concatenation has been done successfully
*/
Standard_EXPORT static  Standard_Integer ConcatenateSurfacesU(const Handle_Geom_BSplineSurface& theS1,
  const Handle_Geom_BSplineSurface& theS2,const Standard_Real theTol,Handle_Geom_BSplineSurface& theS) ;

//! Concatenate two surfaces along any coinciding boundaries
/*! 
	\param theS1 the first surface
  \param theS1 the second surface
  \param theTol the value of sewing tolerance
  \param theS resulting surface 
  \return 0 if the concatenation has been done successfully
*/

Standard_EXPORT static Standard_Integer ConcatenateSurfaces(const Handle(Geom_BSplineSurface)& theS1,
  const Handle(Geom_BSplineSurface)& theS2,
  const Standard_Real theTol,
  Handle(Geom_BSplineSurface)& theS);

//! Concatenate sequence of surfaces along any coinciding boundaries
/*! Two coherent surfaces must have common boundary inside tolerance theTol
	\param theSurfaces the sequences of surfaces, 
  !two cosequence surfaces must have common boundary with tolerance theTol
  \param theTol the value of sewing tolerance
  \param theS resulting surface 
  \return 0 if the concatenation has been done successfully
*/

Standard_EXPORT static Standard_Integer ConcatenateSurfaces(
  const TColGeom_SequenceOfSurface& theSurfaces,
  const Standard_Real theTol,
  Handle(Geom_BSplineSurface)& theS);

protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 


 // Fields PRIVATE
 //


};





// other Inline functions and methods (like "C++: function call" methods)
//


#endif
