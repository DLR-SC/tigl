//copyright, lizenz


#include "generated/CPACSCProfileGeometry.h"
#include "generated/CPACSCNacaProfile.h"
#include "tigl_internal.h"
#include "ITiglWingProfileAlgo.h"
#include "Cache.h"

#include <vector>
#include <TopoDS_Edge.hxx>

#pragma once

namespace tigl{

    class CTiglWingProfileNACA : public ITiglWingProfileAlgo
    {
    public:
        CTiglWingProfileNACA(const CCPACSWingProfile& profile, const std::string& naca_code);
        //an cst implementierung orientiern,
        
        // Update of wire points ...
        TIGL_EXPORT void Invalidate() const override;
        
        // Returns the profile points as read from TIXI.
        TIGL_EXPORT const std::vector<CTiglPoint>& GetSamplePoints() const override; // TODO: why do we need those anyway, they just return an empty vector?

        // get upper wing profile wire
        TIGL_EXPORT const TopoDS_Edge& GetUpperWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const override;

        // get lower wing profile wire
        TIGL_EXPORT const TopoDS_Edge& GetLowerWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const override;

        // get trailing edge
        TIGL_EXPORT const TopoDS_Edge& GetTrailingEdge(TiglShapeModifier mod = UNMODIFIED_SHAPE) const override;

        // gets the upper and lower wing profile into on edge
        TIGL_EXPORT const TopoDS_Edge& GetUpperLowerWire(TiglShapeModifier mod = UNMODIFIED_SHAPE) const override;

        // get leading edge point();
        TIGL_EXPORT const gp_Pnt & GetLEPoint() const override;

        // get trailing edge point();
        TIGL_EXPORT const gp_Pnt & GetTEPoint() const override;

        // CST profiles have always sharp trailing edges
        TIGL_EXPORT bool HasBluntTE() const override;



    private:
        struct WireCache {
        TopoDS_Edge               upperWireOpened;      /**< wire of the upper wing profile */
        TopoDS_Edge               lowerWireOpened;      /**< wire of the lower wing profile */
        TopoDS_Edge               upperWireClosed;      /**< wire of the upper wing profile */
        TopoDS_Edge               lowerWireClosed;      /**< wire of the lower wing profile */
        TopoDS_Edge               upperLowerEdgeOpened; /**< edge consisting of upper and lower wing profile */
        TopoDS_Edge               upperLowerEdgeClosed; /**< edge consisting of upper and lower wing profile */ 
        TopoDS_Edge               trailingEdgeOpened;   /**< edge of the trailing edge */
        TopoDS_Edge               trailingEdgeClosed;   /**< edge of the trailing edge */
        gp_Pnt                    lePoint;              /**< Leading edge point */
        gp_Pnt                    tePoint;              /**< Trailing edge point */
    };

    void InvalidateParent() const;

    // Builds the wing profile wires.
    void BuildNaca(NacaCache& cache) const;

    private:
        Cache<NacaCache, CCPACSWingProfileNACA> wireCache;

    };
}