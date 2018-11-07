/* 
* Copyright (C) 2016 Airbus Defence and Space
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
#ifndef CCPACSWINGSPARSEGMENT_H
#define CCPACSWINGSPARSEGMENT_H


#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>

#include "generated/CPACSSparSegment.h"
#include "CCPACSWingSparPositionUIDs.h"
#include "tigl.h"
#include "CTiglAbstractGeometricComponent.h"
#include "Cache.h"

// forward declarations
class gp_Pnt;
class gp_Vec;


namespace tigl
{

// forward declarations
class CCPACSWingSpars;
class CCPACSWingSparPosition;


class CCPACSWingSparSegment : public generated::CPACSSparSegment, public CTiglAbstractGeometricComponent
{
public:
    enum SparCapSide
    {
        UPPER,
        LOWER
    };

public:
    TIGL_EXPORT CCPACSWingSparSegment(CCPACSWingSparSegments* sparSegments, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT CCPACSWingSparPosition& GetSparPosition(std::string);
    TIGL_EXPORT const CCPACSWingSparPosition& GetSparPosition(std::string) const;

    TIGL_EXPORT gp_Pnt GetPoint(double sparEta) const;

    TIGL_EXPORT gp_Vec GetDirection(double sparEta) const;

    TIGL_EXPORT double GetSparLength() const;

    TIGL_EXPORT gp_Pnt GetMidplanePoint(int positionIndex) const;

    TIGL_EXPORT void GetEtaXsi(int positionIndex, double& eta, double& xsi) const;

    TIGL_EXPORT TopoDS_Wire GetSparMidplaneLine() const;

    TIGL_EXPORT TopoDS_Shape GetSparGeometry(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    TIGL_EXPORT TopoDS_Shape GetSplittedSparGeometry(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    TIGL_EXPORT TopoDS_Shape GetSparCutGeometry(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    TIGL_EXPORT bool HasCap(SparCapSide side) const;

    TIGL_EXPORT TopoDS_Shape GetSparCapsGeometry(SparCapSide side, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // ---------- INTERFACE OF CTiglAbstractGeometricComponent ------------- //
    TIGL_EXPORT virtual std::string GetDefaultedUID() const OVERRIDE;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT virtual TiglGeometricComponentType GetComponentType() const OVERRIDE;

private:
    struct AuxiliaryGeomCache
    {
        TopoDS_Wire sparMidplaneLine;
        TopoDS_Shape sparCutShape;
    };

    struct GeometryCache
    {
        TopoDS_Shape shape;
    };

    struct SplittedGeomCache
    {
        TopoDS_Shape shape;
    };

    struct SparCapsCache
    {
        TopoDS_Shape upperCapsShape;
        TopoDS_Shape lowerCapsShape;
    };

    void BuildAuxiliaryGeometry(AuxiliaryGeomCache& cache) const; // Builds the cutting geometry for the spar as well as the midplane line
    void BuildGeometry(GeometryCache& cache) const;
    void BuildSplittedSparGeometry(SplittedGeomCache& cache) const; // Builds the spar geometry splitted with the ribs
    void BuildSparCapsGeometry(SparCapsCache& cache) const;

    gp_Pnt GetMidplanePoint(const std::string& positionUID) const;

    gp_Vec GetUpVector(const std::string& positionUID, gp_Pnt midplanePnt) const;

    PNamedShape BuildLoft() const OVERRIDE;

private:
    CCPACSWingSparSegment(const CCPACSWingSparSegment&);
    void operator=(const CCPACSWingSparSegment&);

private:
    CCPACSWingSpars& sparsNode;

    Cache<AuxiliaryGeomCache, CCPACSWingSparSegment> auxGeomCache;
    Cache<GeometryCache, CCPACSWingSparSegment> geometryCache;
    Cache<SplittedGeomCache, CCPACSWingSparSegment> splittedGeomCache;
    Cache<SparCapsCache, CCPACSWingSparSegment> sparCapsCache;
};

/** 
 * Tests whether the test point is in front of the spar segment geometry or behind.
 * 
 * The reference axis is given in normal.
 */
TIGL_EXPORT bool PointIsInfrontSparGeometry(gp_Dir normal, gp_Pnt point, TopoDS_Shape sparGeometry);

} // end namespace tigl

#endif // CCPACSWINGSPARSEGMENT_H
