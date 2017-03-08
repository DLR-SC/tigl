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

#include <string>

#include <tixi.h>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>

#include "CCPACSWingSparPositionUIDs.h"
#include "CCPACSWingSparCrossSection.h"
#include "tigl.h"


// forward declarations
class gp_Pnt;
class gp_Vec;


namespace tigl
{

// forward declarations
class CCPACSWingSpars;
class CCPACSWingSparPosition;


class CCPACSWingSparSegment
{
public:
    enum SparCapSide
    {
        UPPER,
        LOWER
    };

public:
    TIGL_EXPORT CCPACSWingSparSegment(CCPACSWingSpars* sparsNode);
    TIGL_EXPORT virtual ~CCPACSWingSparSegment(void);

    TIGL_EXPORT void Invalidate(void);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & sparSegmentXPath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & sparSegmentXPath);

    TIGL_EXPORT const std::string & GetUID(void) const;
    TIGL_EXPORT const std::string& GetName() const;
    TIGL_EXPORT const std::string& GetDescription() const;

    TIGL_EXPORT int GetSparPositionUIDCount() const;
    TIGL_EXPORT const std::string& GetSparPositionUID(int) const;
    TIGL_EXPORT CCPACSWingSparPosition& GetSparPosition(std::string) const;

    TIGL_EXPORT const CCPACSWingSparCrossSection& GetSparCrossSection() const;
    TIGL_EXPORT CCPACSWingSparCrossSection& GetSparCrossSection();

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

protected:
    void Cleanup(void);

    // Builds the cutting geometry for the spar as well as the midplane line
    void BuildAuxiliaryGeometry() const;

    void BuildGeometry() const;

    // Builds the spar geometry splitted with the ribs
    void BuildSplittedSparGeometry() const;

    void BuildSparCapsGeometry() const;

    gp_Pnt GetMidplanePoint(const std::string& positionUID) const;

    gp_Vec GetUpVector(const std::string& positionUID, gp_Pnt midplanePnt) const;

private:
    CCPACSWingSparSegment(const CCPACSWingSparSegment&);
    void operator=(const CCPACSWingSparSegment&);

    struct AuxiliaryGeomCache
    {
        bool valid;
        TopoDS_Wire sparMidplaneLine;
        TopoDS_Shape sparCutShape;
    };

    struct GeometryCache
    {
        bool valid;
        TopoDS_Shape shape;
    };

    struct SplittedGeomCache
    {
        bool valid;
        TopoDS_Shape shape;
    };

    struct SparCapsCache
    {
        bool valid;
        TopoDS_Shape upperCapsShape;
        TopoDS_Shape lowerCapsShape;
    };

private:
    CCPACSWingSpars& sparsNode;
    std::string uid;
    std::string description;
    std::string name;
    CCPACSWingSparPositionUIDs sparPositionUIDs;
    CCPACSWingSparCrossSection sparCrossSection;

    mutable AuxiliaryGeomCache auxGeomCache;
    mutable GeometryCache geometryCache;
    mutable SplittedGeomCache splittedGeomCache;
    mutable SparCapsCache sparCapsCache;
};

} // end namespace tigl

#endif // CCPACSWINGSPARSEGMENT_H
