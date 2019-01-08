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
#ifndef CCPACSWINGRIBSDEFINITION_H
#define CCPACSWINGRIBSDEFINITION_H

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>

#include <vector>

#include "generated/CPACSWingRibsDefinition.h"
#include "CTiglAbstractGeometricComponent.h"
#include "Cache.h"

namespace tigl
{
class CCPACSWingCSStructure;

class CCPACSWingRibsDefinition : public generated::CPACSWingRibsDefinition, public CTiglAbstractGeometricComponent
{
public:
    /**
    * Struct for storing cut geometry, or if necessary the final geometry
    */
    struct CutGeometry
    {
        CutGeometry(const TopoDS_Face& face, bool targetFace) : shape(face), isTargetFace(targetFace) {};

        TopoDS_Face shape;
        bool isTargetFace;
    };

    enum RibCapSide
    {
        UPPER,
        LOWER
    };

    enum RibPositioningType
    {
        RIB_EXPLICIT_POSITIONING,
        RIBS_POSITIONING,
        UNDEFINED_POSITIONING
    };

public:
    TIGL_EXPORT CCPACSWingRibsDefinition(CCPACSWingRibsDefinitions* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT RibPositioningType GetRibPositioningType() const;

    // Returns the number of ribs for this rib definition, either this is
    // defined directly in the ribsPositioning or otherwise it is computed based
    // on the spacing defined in the ribsPositioning
    TIGL_EXPORT int GetNumberOfRibs() const;

    // Returns the start and end point of the rib with the passed index in the 
    // midplane
    TIGL_EXPORT void GetRibMidplanePoints(int ribNumber, gp_Pnt& startPnt, gp_Pnt& endPnt) const;

    // Returns the shape of the Ribs (either single rib or a compound of 
    // multiple ribs)
    TIGL_EXPORT TopoDS_Shape GetRibsGeometry(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Returns the CutGeometry for the rib with the passed number, which can be
    // the cut face for the rib or the final rib face already
    // NOTE: The shapes in the returned CutGeometry are defined relative to the
    //       WING_COORDINATE_SYSTEM
    TIGL_EXPORT const CutGeometry& GetRibCutGeometry(int ribNumber) const;

    // Returns the rib geometry of a single rib
    TIGL_EXPORT TopoDS_Face GetRibFace(int ribNumber, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Returns the rib geometry splitted with the spars
    TIGL_EXPORT TopoDS_Shape GetSplittedRibsGeometry(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Returns whether rib caps are defined or not
    TIGL_EXPORT bool HasCaps() const;
    TIGL_EXPORT TopoDS_Shape GetRibCapsGeometry(RibCapSide side, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // ---------- INTERFACE OF CTiglAbstractGeometricComponent ------------- //
    TIGL_EXPORT virtual std::string GetDefaultedUID() const OVERRIDE;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT virtual TiglGeometricComponentType GetComponentType() const OVERRIDE;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE;

protected:
    PNamedShape BuildLoft() const OVERRIDE;

private:
    // Structure containing data relevant when ribs are defined via 
    // ribsPositioning (defining a set or ribs)
    struct RibSetDataCache
    {
        int numberOfRibs;
        double referenceEtaStart;
        double referenceEtaEnd;
        double referenceEtaOffset;
    };

    // Structure for storing the start and end point of a single rib in the 
    // midplane
    struct RibMidplanePoints
    {
        RibMidplanePoints(const gp_Pnt& startPnt, const gp_Pnt& endPnt) : startPnt(startPnt), endPnt(endPnt) {};
        gp_Pnt startPnt;
        gp_Pnt endPnt;
    };

    // Structure for storing the start and end points of all ribs and the cut
    // shapes for the single ribs
    struct AuxiliaryGeomCache
    {
        std::vector<RibMidplanePoints> midplanePoints;
        std::vector<CutGeometry> cutGeometries;
    };

    struct RibGeometryCache
    {
        TopoDS_Shape shape;
        std::vector<TopoDS_Face> ribFaces;
    };

    struct SplittedRibGeometryCache
    {
        TopoDS_Shape shape;
    };

    struct RibCapsGeometryCache
    {
        TopoDS_Shape upperCapsShape;
        TopoDS_Shape lowerCapsShape;
    };

    void UpdateRibSetDataCache(RibSetDataCache& cache) const;

    void BuildAuxiliaryGeometry(AuxiliaryGeomCache& cache) const;
    void BuildAuxGeomRibsPositioning(AuxiliaryGeomCache& cache) const;
    void BuildAuxGeomExplicitRibPositioning(AuxiliaryGeomCache& cache) const;

    void BuildGeometry(RibGeometryCache& cache) const;

    void BuildSplittedRibsGeometry(SplittedRibGeometryCache& cache) const;

    void BuildRibCapsGeometry(RibCapsGeometryCache& cache) const;

    // Generates the cut shape for the rib at the passed eta position, or at the passed element or spar position
    CutGeometry BuildRibCutGeometry(double currentEta, const std::string& elementUID, const std::string& sparPositionUID, AuxiliaryGeomCache& cache) const;

    // Returns the wire of the rib reference line (either leadingEdge, 
    // trailingEdge, or spar midplane line)
    TopoDS_Wire GetReferenceLine() const;

    // Returns the start eta value on the reference line, either directly
    // defined in positioning or it will be computed based on the section
    // element UID
    double ComputeReferenceEtaStart() const;

    // Returns the end eta value on the reference line, either directly defined
    // in positioning or it will be computed based on the section element UID
    double ComputeReferenceEtaEnd() const;

    // Computes the reference-line eta value by intersection of the reference
    // line with the section element
    double ComputeSectionElementEta(const std::string& sectionElementUID) const;

    // Computes the reference-line eta value by intersection of the reference
    // line with the midplane point of the spar position
    double ComputeSparPositionEta(const std::string& sparPositionUID) const;

    // Returns the number of ribs in case the RibsPositioning is used by
    // computing the values from the ribsPositioning
    // NOTE: may only called when using ribsPositioning!!!
    int ComputeNumberOfRibs(double etaStart, double etaEnd) const;

    // Returns the eta difference between two ribs in the ribs definition
    double ComputeEtaOffset(double etaStart, double etaEnd) const;

    // returns the rib geometry inside a section
    // The elementUID value determines which section element should be used.
    // When no elementUID is passed the inner or outer section are used
    // depending on the passed eta value.
    TopoDS_Face GetSectionRibGeometry(const std::string& elementUID, double eta, const std::string& ribStart,
                                      const std::string& ribEnd) const;

    // Computes the direction vector for the rib
    gp_Vec GetRibDirection(double currentEta, const gp_Pnt& startPnt, const gp_Vec& upVec) const;

    // builds the rib cut face
    TopoDS_Face BuildRibCutFace(const gp_Pnt& startPnt, const gp_Vec& ribDir, const std::string& ribStart,
                                const std::string& ribEnd, const gp_Vec& upVecStart, const gp_Vec& upVecEnd) const;

    // Computes the start and end point of the ribs definition
    RibMidplanePoints ComputeRibDefinitionPoints(const std::string& ribStart, const std::string& ribEnd, 
                                                 const TopoDS_Face& ribCutFace) const;

    CCPACSWingCSStructure& getStructure();
    const CCPACSWingCSStructure& getStructure() const;

private:
    CCPACSWingRibsDefinition(const CCPACSWingRibsDefinition&); // = delete;
    void operator=(const CCPACSWingRibsDefinition& ); // = delete;

private:
    Cache<RibSetDataCache, CCPACSWingRibsDefinition> ribSetDataCache;
    Cache<AuxiliaryGeomCache, CCPACSWingRibsDefinition> auxGeomCache;
    Cache<RibGeometryCache, CCPACSWingRibsDefinition> ribGeometryCache;
    Cache<SplittedRibGeometryCache, CCPACSWingRibsDefinition> splittedRibGeomCache;
    Cache<RibCapsGeometryCache, CCPACSWingRibsDefinition> ribCapsCache;
};

} // end namespace tigl

#endif // CCPACSWINGRIBSDEFINITION_H
