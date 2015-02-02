/* 
* This file was created by OpenCASCADE for the German Aerospace Center.
* It will reside in TiGL, until this Algorithm becomes part of
* OpenCASCADE.
* 
* For now, the file will be licensed under the Apache License, Version 2.0.
*    http://www.apache.org/licenses/LICENSE-2.0
*/

#ifndef _MakePatches_HeaderFile
#define _MakePatches_HeaderFile

#ifndef _TopoDS_Shape_HeaderFile
#include <TopoDS_Shape.hxx>
#endif
#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif
#ifndef _GeomFill_FillingStyle_HeaderFile
#include <GeomFill_FillingStyle.hxx>
#endif
#ifndef _Handle_Geom_Surface_HeaderFile
#include <Handle_Geom_Surface.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class TopoDS_Shape;
class Handle_Geom_BSplineSurface;
class TColGeom_SequenceOfBoundedCurve;

/**
 * Implementation of algorithm for filling 4-sides closed frames\n
 * of edges with Coons surfaces to get faces.
 * Then faces can be joined in shell
 */

class MakePatches  {

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

/**
 * Empty constructop
 */

Standard_EXPORT MakePatches();

/**
 * Constructor with initialisation of data
 * @param theGuides
 * compound consisting of guide edges or wires.
 * @param theProfiles
 * compound consisting of profile (section) edges or wires
 */

Standard_EXPORT MakePatches(const TopoDS_Shape& theGuides, const TopoDS_Shape& theProfiles);

/**
 * Method for initialisation of data
 * @param theGuides
 * compound consisting of guide edges or wires.
 * @param theProfiles
 * compound consisting of profile (section) edges or wires
 */

Standard_EXPORT   void Init(const TopoDS_Shape& theGuides, const TopoDS_Shape& theProfiles) ;

/**
 * Perform building patches
 * @param theTolConf
 * Absolute tolerance for checking coincidence of ends of curves.\n
 * This tolerance is used also by sewing algorithm
 * @param theTolParam
 * Relative tolerance for comparing knots of opposite curves\n
 * If k1 and k2 are values of corresponding knots of opposite curves C1 and C2 \n
 * then k1 ~= k2 if Abs(k1-k2)/((k1+k2)/2) <= theTolParam
 * @param theStyle 
 * The style of filling, to provide C2 continuity between patches it is necessary using\n
 * GeomFill_CoonsC2Style
 * @param theSewing
 * Boolean flag that enable/disable sewing of patches
 */

Standard_EXPORT   void Perform(const Standard_Real theTolConf, 
                               const Standard_Real theTolParam, 
                               const GeomFill_FillingStyle theStyle = GeomFill_CoonsC2Style,
                               const Standard_Boolean theSewing = Standard_True) ;

/**
 * Returns result
 * Result can be compound pf faces or shell depending on value of theSewing
 */

Standard_EXPORT  const TopoDS_Shape& Patches() const;

/**
 * Method for building surface by filling 4-side closed contour consisting of 4 curves
 * @param theCurves
 * The sequence of curves for storing closed contour of curves
 * @param theTolConf
 * Absolute tolerance for checking coincidence of ends of curves.
 * @param theTolParam
 * Relative tolerance for comparing knots of opposite curves\n
 * If k1 and k2 are values of corresponding knots of opposite curves C1 and C2 \n
 * then k1 ~= k2 if Abs(k1-k2)/((k1+k2)/2) <= theTolParam
 * @param theBaseCurve
 * The index in sequence theCurves, which define first curve in contour\n
 * this index has effect on origin and UV direction of result surface 
 * @param theStyle 
 * The style of filling, to provide C2 continuity between patches it is necessary using\n
 * GeomFill_CoonsC2Style
 */

Standard_EXPORT static  Handle_Geom_BSplineSurface 
  BuildSurface(const TColGeom_SequenceOfBoundedCurve& theCurves,
               const Standard_Real theTolConf,
               const Standard_Real theTolParam,
               const Standard_Integer theBaseCurve = 1,
               const GeomFill_FillingStyle theStyle = GeomFill_CoonsC2Style);
/**
 * Returns status of calculations\n
 * 0 - success, 1 - algorithm failed 
 */

Standard_EXPORT Standard_Integer GetStatus() const;






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
TopoDS_Shape myGuides;
TopoDS_Shape myProfiles;
TopoDS_Shape myGrid;
TopoDS_Shape myPatches;
Standard_Integer myStatus;


};





// other Inline functions and methods (like "C++: function call" methods)
//


#endif
