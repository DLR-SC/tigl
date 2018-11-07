/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSWINGCELL_H
#define CCPACSWINGCELL_H

#include "ITiglGeometricComponent.h"
#include "generated/CPACSWingCell.h"
#include "generated/UniquePtr.h"
#include "tigletaxsifunctions.h"
#include "Cache.h"

#include <gp_Pln.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <vector>


namespace tigl
{

// forward declarations
class CCPACSWingCells;
class CCPACSWingCellPositionChordwise;
class CCPACSWingCellPositionSpanwise;

class CCPACSWingCell : public generated::CPACSWingCell, public ITiglGeometricComponent
{
public:
    TIGL_EXPORT CCPACSWingCell(CCPACSWingCells* parentCells, CTiglUIDManager* uidMgr);
    TIGL_EXPORT ~CCPACSWingCell() OVERRIDE;

    TIGL_EXPORT void Invalidate();

    // determines if a given eta xsi coordinate is inside this cell
    // TODO: missing support for spar cell borders
    TIGL_EXPORT bool IsInside(double eta, double xsi) const;

    // determines if the cell defines a convex qudriangle or not
    // TODO: missing support for spar cell borders
    TIGL_EXPORT bool IsConvex() const;

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& cellXPath) OVERRIDE;

    // get corner coordinates of cell
    TIGL_EXPORT EtaXsi GetLeadingEdgeInnerPoint() const;
    TIGL_EXPORT EtaXsi GetLeadingEdgeOuterPoint() const;
    TIGL_EXPORT EtaXsi GetTrailingEdgeInnerPoint() const;
    TIGL_EXPORT EtaXsi GetTrailingEdgeOuterPoint() const;

    // sets corner coordinates of cell
    TIGL_EXPORT void SetLeadingEdgeInnerPoint(double eta, double xsi);
    TIGL_EXPORT void SetLeadingEdgeOuterPoint(double eta, double xsi);
    TIGL_EXPORT void SetTrailingEdgeInnerPoint(double eta, double xsi);
    TIGL_EXPORT void SetTrailingEdgeOuterPoint(double eta, double xsi);

    // set cell borders via ribs or spars
    TIGL_EXPORT void SetLeadingEdgeSpar(const std::string& sparUID);
    TIGL_EXPORT void SetTrailingEdgeSpar(const std::string& sparUID);
    TIGL_EXPORT void SetInnerBorderRib(const std::string& ribDefinitionUID, int ribNumber);
    TIGL_EXPORT void SetOuterBorderRib(const std::string& ribDefinitionUID, int ribNumber);

    TIGL_EXPORT CCPACSMaterialDefinition& GetMaterial();
    TIGL_EXPORT const CCPACSMaterialDefinition& GetMaterial() const;

    TIGL_EXPORT TopoDS_Shape GetSkinGeometry(TiglCoordinateSystem cs = GLOBAL_COORDINATE_SYSTEM) const;

    TIGL_EXPORT bool IsPartOfCell(TopoDS_Face);
    TIGL_EXPORT bool IsPartOfCell(TopoDS_Edge);

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;
    TIGL_EXPORT PNamedShape GetLoft() const OVERRIDE;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE;

private:
    struct EtaXsiCache
    {
        EtaXsi innerLeadingEdgePoint;
        EtaXsi innerTrailingEdgePoint;
        EtaXsi outerLeadingEdgePoint;
        EtaXsi outerTrailingEdgePoint;
    };

    struct GeometryCache
    {
        TopoDS_Shape skinGeometry;

        gp_Pln cutPlaneLE, cutPlaneTE, cutPlaneIB, cutPlaneOB;
        TopoDS_Shape planeShapeLE, planeShapeTE, planeShapeIB, planeShapeOB;
        TopoDS_Shape sparShapeLE, sparShapeTE;
        gp_Pnt projectedIBLE, projectedOBLE, projectedIBTE, projectedOBTE;
    };

    template<class T>
    bool IsPartOfCellImpl(T t);
    
    EtaXsi computePositioningEtaXsi(const CCPACSWingCellPositionSpanwise& spanwisePos,
                                    const CCPACSWingCellPositionChordwise& chordwisePos, bool inner,
                                    bool front) const;

    // calculates the Eta/Xsi values of the the cell's corner points and stores
    // them in the cache
    void UpdateEtaXsiValues(EtaXsiCache& cache) const;

    void Reset();

    void BuildSkinGeometry(GeometryCache& cache) const;

    TopoDS_Shape GetRibCutGeometry(std::pair<std::string, int> ribUidAndIndex) const;

    Cache<EtaXsiCache, CCPACSWingCell> m_etaXsiCache;
    Cache<GeometryCache, CCPACSWingCell> m_geometryCache;
};

namespace WingCellInternal
{
    struct Point2D
    {
        double x;
        double y;
    };

    // calculates crossproduct (p1-p3)x(p2-p3) (only "z"-value)
    TIGL_EXPORT double sign(Point2D p1, Point2D p2, Point2D p3);

    // calculates the area of a triangle
    TIGL_EXPORT double area(Point2D p1, Point2D p2, Point2D p3);

    // checks if point p is in triangle p1-p2-p3
    TIGL_EXPORT bool is_in_trian(Point2D p, Point2D p1, Point2D p2, Point2D p3);
}

} // namespace tigl

#endif // CCPACSWINGCELL_H
