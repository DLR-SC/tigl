/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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

#ifndef CTIGLINTERSECTIONCALCULATIONBASE
#define CTIGLINTERSECTIONCALCULATIONBASE

#include "tigl.h"
#include "tigl_internal.h"
#include "CCPACSWings.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"

#include "GeomAPI_IntSS.hxx"
#include "ShapeAnalysis_Wire.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"


namespace tigl
{

class CTiglShapeCache;

class CTiglIntersectionCalculationBase
{

public:
    // Constructor, load intersection result from cache
    // cache is mandatory, hence the reference
    TIGL_EXPORT CTiglIntersectionCalculationBase(CTiglShapeCache& cache,
                                             const std::string& intersectionID);
    // Destructor
    TIGL_EXPORT virtual ~CTiglIntersectionCalculationBase();

    // returns total number of intersection lines
    TIGL_EXPORT int GetCountIntersectionLines();

    // returns total number of intersection points
    TIGL_EXPORT int GetCountIntersectionPoints();

    // returns total number of intersection faces
    TIGL_EXPORT int GetCountIntersectionFaces();

    // Gets a point on the intersection line in dependence of a parameter zeta with
    // 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the line starting point,
    // for zeta = 1.0 the last point on the intersection line.
    // numIntersecLine is the number of the Intersection line.
    TIGL_EXPORT gp_Pnt GetPoint(double zeta, int wireID);

    // gives a reference to the computed wire
    TIGL_EXPORT TopoDS_Wire GetWire(int wireID);

    // gives a reference to the computed vertex
    TIGL_EXPORT TopoDS_Vertex GetVertex(int vertexID);

    //gives a reference to the computed area
    TIGL_EXPORT TopoDS_Face GetFace(int faceID);

    // returnes the unique ID for the current intersection
    TIGL_EXPORT const std::string& GetID();

private:
    Standard_Real tolerance;
    TopoDS_Compound intersectionResult;     /* The full Intersection result */
    std::string id;                         /* identifcation id of the intersection */

};

} // end namespace tigl

#endif // CTIGLINTERSECTIONCALCULATIONBASE
