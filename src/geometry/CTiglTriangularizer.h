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

class TopoDS_Shape;
class TopoDS_Face;

namespace tigl 
{
class CTiglRelativeComponent;
class CCPACSConfiguration;
class CCPACSWingSegment;

enum ComponentTraingMode 
{
    NO_INFO,
    SEGMENT_INFO
};

class CTiglTriangularizer : public CTiglPolyData
{
public:
    TIGL_EXPORT CTiglTriangularizer();
    TIGL_EXPORT CTiglTriangularizer(const TopoDS_Shape&, double deflection, bool useMultipleObjects = false);
    TIGL_EXPORT CTiglTriangularizer(CTiglRelativeComponent& comp, double deflection, ComponentTraingMode mode);
    TIGL_EXPORT CTiglTriangularizer(CCPACSConfiguration& config, bool fuseShapes, double deflection, ComponentTraingMode mode);
    
    TIGL_EXPORT void useMultipleObjects(bool);
    
private:
    int triangularizeComponent(CTiglRelativeComponent& component, bool includeChilds, const TopoDS_Shape& shape, double deflection, ComponentTraingMode = NO_INFO);
    int triangularizeComponent(const std::vector<CTiglRelativeComponent*>& components, bool includeChilds, const TopoDS_Shape& shape, double deflection, ComponentTraingMode = NO_INFO);
    int triangularizeShape(const TopoDS_Shape&);
    void annotateWingSegment(CCPACSWingSegment& segment, gp_Pnt centralP, bool pointOnMirroredShape, unsigned long iPolyLow, unsigned long iPolyUp);
    int triangularizeFace(const TopoDS_Face&, unsigned long& nVertices, unsigned long& iPolyLow, unsigned long& iPolyUp);
    
    bool _useMultipleObjects;
};

}

#endif // CTIGLTRIANGULARIZER_H
