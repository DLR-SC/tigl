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
class CTiglRelativelyPositionedComponent;
class CCPACSConfiguration;
class CCPACSWingSegment;

enum ComponentTraingMode 
{
    NO_INFO,
    SEGMENT_INFO
};

class CTiglTriangularizerOptions {
public:
    CTiglTriangularizerOptions()
        : m_useMultipleObjects(false),
          m_normalsEnabled(true)
    {
    }

    bool useMultipleObjects() const
    {
        return m_useMultipleObjects;
    }

    bool normalsEnabled() const
    {
        return m_normalsEnabled;
    }

    void setNormalsEnabled(bool enabled)
    {
        m_normalsEnabled = enabled;
    }

    void setMutipleObjectsEnabled(bool enabled)
    {
        m_useMultipleObjects = enabled;
    }

private:
    bool m_useMultipleObjects;
    bool m_normalsEnabled;
};

class CTiglTriangularizer
{
public:
    TIGL_EXPORT CTiglTriangularizer(const TopoDS_Shape&, double deflection, const CTiglTriangularizerOptions& options = CTiglTriangularizerOptions());

    TIGL_EXPORT CTiglTriangularizer(class CTiglRelativelyPositionedComponent &comp, double deflection,
                                    ComponentTraingMode mode, const CTiglTriangularizerOptions& options = CTiglTriangularizerOptions());

    TIGL_EXPORT CTiglTriangularizer(class CCPACSConfiguration& config, bool fuseShapes, double deflection, ComponentTraingMode mode,
                                    const CTiglTriangularizerOptions& options = CTiglTriangularizerOptions());
    
    const CTiglPolyData& getTriangulation() const
    {
        return polys;
    }

private:
    int triangularizeComponent(CTiglRelativelyPositionedComponent& component, bool includeChilds, const TopoDS_Shape& shape, double deflection, ComponentTraingMode = NO_INFO);
    int triangularizeComponent(const std::vector<CTiglRelativelyPositionedComponent*>& components, bool includeChilds, const TopoDS_Shape& shape, double deflection, ComponentTraingMode = NO_INFO);
    int triangularizeShape(const TopoDS_Shape&);
    void annotateWingSegment(CCPACSWingSegment& segment, gp_Pnt centralP, bool pointOnMirroredShape, unsigned long iPolyLow, unsigned long iPolyUp);
    int triangularizeFace(const TopoDS_Face&, unsigned long& nVertices, unsigned long& iPolyLow, unsigned long& iPolyUp);
    int computeVTKMetaData(class CCPACSWing&);

    // some options
    CTiglTriangularizerOptions m_options;
    CTiglPolyData polys;
};

}

#endif // CTIGLTRIANGULARIZER_H
