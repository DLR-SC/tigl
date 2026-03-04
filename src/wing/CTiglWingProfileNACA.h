/* 
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-14 Hannah Gedler <hannah.gedler@dlr.de>
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
* @brief Header for NACA wing profile 
*/


#include "CPACSProfileGeometry.h"
//#include "generated/CPACSNacaProfile.h"
#include "tigl_internal.h"
#include "ITiglWingProfileAlgo.h"
#include "Cache.h"
#include "NACA4Calculator.h"

#include <vector>
#include <TopoDS_Edge.hxx>

#pragma once

namespace tigl{

    class CTiglWingProfileNACA : public ITiglWingProfileAlgo
    {
    public:
        CTiglWingProfileNACA(const CCPACSWingProfile& profile, const generated::CPACSNacaProfile& nacadef);
        
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
        TopoDS_Edge               upperWire;      /**< wire of the upper wing profile */
        TopoDS_Edge               lowerWire;      /**< wire of the lower wing profile */
        // TopoDS_Edge               upperLowerEdge; /**< edge consisting of upper and lower wing profile */
        TopoDS_Edge               trailingEdge; 
        gp_Pnt                    lePoint;              /**< Leading edge point */
        gp_Pnt                    tePoint;              /**< Trailing edge point */
    };

    void InvalidateParent() const;

    // Builds the wing profile wires.
    void BuildWires(WireCache& cache) const;



    private:
       std::string profileUID;
       NACA4Calculator calculator;
       double te_thickness;
       Cache<WireCache, CTiglWingProfileNACA> wireCache;


    };
}