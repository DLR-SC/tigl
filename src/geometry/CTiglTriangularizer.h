/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-18 Martin Siggel <Martin.Siggel@dlr.de>
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
*
* @file CTiglTriangularizer.h
* @brief Helper class to traingularize tigl objects and TopoDS_Shapes
*/

#ifndef CTIGLTRIANGULARIZER_H
#define CTIGLTRIANGULARIZER_H

#include <vector>
#include "tigl_internal.h"
#include "CTiglPolyData.h"
#include <gp_Pnt.hxx>
#include "PNamedShape.h"

class TopoDS_Shape;
class TopoDS_Face;

namespace tigl 
{
class CTiglRelativelyPositionedComponent;
class CCPACSConfiguration;
class CCPACSWingSegment;
class CTiglUIDManager;
class ITiglGeometricComponent;

enum ComponentTraingMode 
{
    NO_INFO,
    SEGMENT_INFO
};

class CTiglTriangularizer
{
public:
    TIGL_EXPORT CTiglTriangularizer(PNamedShape shape, double deflection, bool computeNormals = true);

    TIGL_EXPORT CTiglTriangularizer(const CTiglUIDManager* uidMgr, PNamedShape shape, double deflection,
                                    ComponentTraingMode mode = NO_INFO, bool computeNormals = true);

    const CTiglPolyData& getTriangulation() const
    {
        return polys;
    }

private:
    int triangularizeComponent(const CTiglUIDManager* uidMgr, PNamedShape shape, double deflection, ComponentTraingMode = NO_INFO);
    int triangularizeFace(const TopoDS_Face&, unsigned long& nVertices, unsigned long& iPolyLow, unsigned long& iPolyUp);

    void writeFaceDummyMeta(unsigned long iPolyLower, unsigned long iPolyUpper);
    bool writeWingMeta(ITiglGeometricComponent& wing, gp_Pnt centralP, unsigned long iPolyLower, unsigned long iPolyUpper);
    bool writeWingSegmentMeta(ITiglGeometricComponent& segment, gp_Pnt centralP, unsigned long iPolyLower, unsigned long iPolyUpper);

    // some options
    bool m_computeNormals;
    CTiglPolyData polys;
    void writeFaceMeta(const CTiglUIDManager* uidMgr, const std::string& componentUID, TopoDS_Face face, unsigned long iPolyLower, unsigned long iPolyUpper);
};

}

#endif // CTIGLTRIANGULARIZER_H
