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

#include "tigl_internal.h"
#include "CTiglPolyData.h"
#include <gp_Pnt.hxx>

class TopoDS_Shape;
class TopoDS_Face;

namespace tigl 
{

enum ComponentTraingMode 
{
    NO_INFO,
    SEGMENT_INFO
};

class CTiglTriangularizer : public CTiglPolyData
{
public:
    TIGL_EXPORT CTiglTriangularizer();
    TIGL_EXPORT CTiglTriangularizer(const TopoDS_Shape&, double deflection);
    TIGL_EXPORT CTiglTriangularizer(class CTiglAbstractPhysicalComponent &comp, double deflection, ComponentTraingMode mode);
    TIGL_EXPORT CTiglTriangularizer(class CCPACSConfiguration& config, bool fuseShapes, double deflection, ComponentTraingMode mode);
    
    TIGL_EXPORT static void useMultipleObjects(bool);
    TIGL_EXPORT static void setNormalsEnabled(bool);
    
private:
    int triangularizeComponent(class CTiglAbstractPhysicalComponent &, bool includeChilds, const TopoDS_Shape& shape, double deflection, ComponentTraingMode = NO_INFO);
    int triangularizeShape(const TopoDS_Shape &);
    void annotateWingSegment(class CCPACSWingSegment &segment, gp_Pnt centralP, bool pointOnMirroredShape, unsigned long iPolyLow, unsigned long iPolyUp);
    int triangularizeFace(const TopoDS_Face &, unsigned long& nVertices, unsigned long& iPolyLow, unsigned long& iPolyUp);
    int computeVTKMetaData(class CCPACSWing&);

    // some options
    static bool _useMultipleObjects;
    static bool _normalsEnabled;
};

}

#endif // CTIGLTRIANGULARIZER_H
