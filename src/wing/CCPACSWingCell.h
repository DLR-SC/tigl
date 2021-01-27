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
#include "CTiglRectGridSurface.h"

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
    TIGL_EXPORT ~CCPACSWingCell() override;

    // determines if a given eta xsi coordinate is inside this cell
    // TODO: missing support for spar cell borders
    TIGL_EXPORT bool IsInside(double eta, double xsi) const;

    // determines if the cell defines a convex qudriangle or not
    // TODO: missing support for spar cell borders
    TIGL_EXPORT bool IsConvex() const;

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& cellXPath) override;

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

    TIGL_EXPORT std::string GetDefaultedUID() const override;
    TIGL_EXPORT PNamedShape GetLoft() const override;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override;

private:
    // Invalidates internal state
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

    struct EtaXsiCache
    {
        EtaXsi innerLeadingEdgePoint;
        EtaXsi innerTrailingEdgePoint;
        EtaXsi outerLeadingEdgePoint;
        EtaXsi outerTrailingEdgePoint;
    };

    // This struct is used to annotate all the faces of the
    // wing skin. The annotations are used in the lofting algorithm
    struct TrimShapeAnnotation
    {
        double ccmin, ccmax; // chordwise contour coordinate bounds
        double scmin, scmax; // spanwise contour coordinate bounds
        bool keep = true;    // a label, specifying if the face
                             // will be part of the resulting shape
    };

    struct GeometryCache
    {
        TopoDS_Shape skinGeometry;

        // the following variables are used if the cell is defined
        // relative to the chordface
        gp_Pnt IBLE, projectedIBLE,
               OBLE, projectedOBLE,
               IBTE, projectedIBTE,
               OBTE, projectedOBTE;
        gp_Ax3 border_inner_ax3, border_outer_ax3, border_le_ax3, border_te_ax3;
        TopoDS_Shape sparShapeLE, sparShapeTE;

        // this cache is used for the contour coordinate implementation
        CTiglRectGridSurface<TrimShapeAnnotation> rgsurface;
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

    // this enum is used internally by CutSpanWise to determine,
    // wether it is an inner or an outer cut
    enum class SpanWiseBorder {
        Inner,
        Outer
    };

    /**
     * @brief CutSpanwise cuts the loftShape in spanwise direction
     * Depending on the enum border, it can be a inner or  outer
     * border. This function is called by BuildSKinGeometry
     * to cut the loftSurface in spanwise direction
     *
     * This function is used, if the borders of the cell are defined
     * relative to the chordface via eta xsi coordinates or by
     * referencing a rib
     *
     * @param cache The shape cache of the wing cell
     * @param loftShape the shape of the wing skin
     * @param border a SpanWiseBorder enum, denoting wether it is a inner
     * or outer border
     * @param positioning CPACS definition of the border
     * @param zRefDir a reference direction orthogonal to the chordface
     * @param tol a tolerance
     * @return The loftShape cut at the spanwise border
     */
    TopoDS_Shape CutSpanwise(GeometryCache& cache,
                             TopoDS_Shape const& loftShape,
                             SpanWiseBorder border,
                             CCPACSWingCellPositionSpanwise const& positioning,
                             gp_Dir const& zRefDir,
                             double tol=5e-3) const;

    /**
     * @brief TrimSpanwise trims the wing skin in spanwise direction
     * Depending on the enum border, it can be a inner or outer
     * border. This function is called by BuildSkinGeometry to trim
     * the wing shape in spanwise direction.
     *
     * This function is used, it the borders of the cell are defined
     * relative to the wing skin via contour coordinates.
     *
     * @param cache The shape of the wing cell
     * @param border  a SpanWiseBorder enum, denoting wether it is a inner
     * or outer border
     * @param positioning CPACS definition of the border
     * @param tol a tolerance
     */
    void TrimSpanwise(GeometryCache& cache,
                      SpanWiseBorder border,
                      CCPACSWingCellPositionSpanwise const& positioning,
                      double tol = 5e-3) const;

    // this enum is used internally by CutChordwise to determine,
    // wether it is a leading edge or trailing edge cut
    enum class ChordWiseBorder {
        LE,
        TE
    };

    /**
     * @brief CutChordwise cuts the loftShape in chordwise direction
     * Depending on the enum border, it can be a leading edge or
     * trailing edge border. This function is called by BuildSKinGeometry
     * to cut the loftSurface in chordwise direction
     *
     * This function is used, if the borders of the cell are defined
     * relative to the chordface via eta xsi coordinates or by
     * referencing a spar
     *
     * @param cache The shape cache of the wing cell
     * @param loftShape the shape of the wing skin
     * @param border a ChordWiseBorder enum, denoting wether it is a leading edge
     * or trailing edge border
     * @param positioning CPACS definition of the border
     * @param zRefDir a reference direction orthogonal to the chordface
     * @param tol a tolerance
     * @return The loftShape cut at the chordwise border
     */
    TopoDS_Shape CutChordwise(GeometryCache& cache,
                              TopoDS_Shape const& loftShape,
                              ChordWiseBorder border,
                              CCPACSWingCellPositionChordwise const& positioning,
                              gp_Dir const& zRefDir,
                              double tol=5e-3) const;

    /**
     * @brief TrimChordwise trims the wing skin in chordwise direction
     * Depending on the enum border, it can be a leading edge or trailing edge
     * border. This function is called by BuildSkinGeometry to trim
     * the wing shape in chordwise direction.
     *
     * This function is used, it the borders of the cell are defined
     * relative to the wing skin via contour coordinates.
     *
     * @param cache The shape of the wing cell
     * @param border  a ChordWiseBorder enum, denoting wether it is a LE
     * or a TE border
     * @param positioning CPACS definition of the border
     * @param tol a tolerance
     */
    void TrimChordwise(GeometryCache& cache,
                       ChordWiseBorder border,
                       CCPACSWingCellPositionChordwise const& positioning,
                       double tol = 5e-3) const;


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
