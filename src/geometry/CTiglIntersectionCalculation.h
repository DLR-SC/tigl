/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file 
* @brief  Class to compute intersection algorithms.
*/

#ifndef CTIGLINTERSECTIONCALCULATION
#define CTIGLINTERSECTIONCALCULATION

#include "tigl.h"
#include "tigl_internal.h"
#include "CCPACSHeader.h"
#include "CCPACSWings.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"

#include "GeomAPI_IntSS.hxx"
#include "ShapeAnalysis_Wire.hxx"
#include "Handle_TopTools_HSequenceOfShape.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"


namespace tigl 
{

class CTiglShapeCache;

class CTiglIntersectionCalculation
{

public:
    // Constructor, compute intersection of two TopoDS_Shapes
    // cache variable can be NULL
    TIGL_EXPORT CTiglIntersectionCalculation(CTiglShapeCache* cache,
                                             const std::string& idOne,
                                             const std::string& idTwo,
                                             TopoDS_Shape compoundOne,
                                             TopoDS_Shape compoundTwo );

    // Computes the intersection of a shape with a plane
    // cache variable can be NULL
    TIGL_EXPORT CTiglIntersectionCalculation(CTiglShapeCache* cache,
                                             const std::string& shapeID,
                                             TopoDS_Shape shape,
                                             gp_Pnt point,
                                             gp_Dir normal );

    // Constructor, load intersection result from cache
    // cache is mandatory, hence the reference
    TIGL_EXPORT CTiglIntersectionCalculation(CTiglShapeCache& cache,
                                             const std::string& intersectionID);

    // Destructor
    TIGL_EXPORT virtual ~CTiglIntersectionCalculation();

    // returns total number of intersection lines
    TIGL_EXPORT int GetCountIntersectionLines();

    // Gets a point on the intersection line in dependence of a parameter zeta with
    // 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the line starting point,
    // for zeta = 1.0 the last point on the intersection line.
    // numIntersecLine is the number of the Intersection line.
    TIGL_EXPORT gp_Pnt GetPoint(double zeta, int wireID);

    // gives a reference to the computed wire
    TIGL_EXPORT TopoDS_Wire GetWire(int wireID);

    // returnes the unique ID for the current intersection
    TIGL_EXPORT const std::string& GetID();

protected:
    void computeIntersection(CTiglShapeCache* cache,
                             size_t hashOne,
                             size_t hashTwo,
                             TopoDS_Shape compoundOne,
                             TopoDS_Shape compoundTwo );

private:        
    Standard_Real tolerance;
    TopoDS_Compound intersectionResult;     /* The full Intersection result */
    std::string id;                         /* identifcation id of the intersection */
    
};

} // end namespace tigl

#endif // CTIGLINTERSECTIONCALCULATION
