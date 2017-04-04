/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
* @brief  Implementation of CPACS wing ComponentSegment handling routines.
*/

#ifndef CCPACSWINGCOMPONENTSEGMENT_H
#define CCPACSWINGCOMPONENTSEGMENT_H

#include "generated/CPACSComponentSegment.h"

#include <Geom_BSplineSurface.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>

#include <tixi.h>
#include "tigl_config.h"
#include "tigl_internal.h"

#include "CCPACSMaterial.h"
#include "CCPACSWingConnection.h"
#include "CCPACSWingCSStructure.h"
#include "CCPACSWingShell.h"
#include "CTiglAbstractSegment.h"
#include "CTiglPoint.h"
#include "CTiglPointTranslator.h"
#include "CCPACSTransformation.h"


namespace tigl
{

class CCPACSWing;
class CCPACSWingSegment;

typedef std::vector<const CCPACSMaterial*>    MaterialList;
typedef std::vector<CCPACSWingSegment*>       SegmentList;

class CCPACSWingComponentSegment : public generated::CPACSComponentSegment, public CTiglAbstractSegment<CCPACSWingComponentSegment>
{
public:
    // Constructor
    TIGL_EXPORT CCPACSWingComponentSegment(CCPACSWingComponentSegments* parent);
    TIGL_EXPORT CCPACSWingComponentSegment(CCPACSWing* aWing, int aSegmentIndex);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWingComponentSegment();

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS segment elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & segmentXPath);

    TIGL_EXPORT virtual const std::string& GetUID() const;
    TIGL_EXPORT virtual void SetUID(const std::string& uid);

    TIGL_EXPORT CCPACSWing& GetWing() const;

    // Getter for upper Shape
    TIGL_EXPORT TopoDS_Shape GetUpperShape();

    // Getter for lower Shape
    TIGL_EXPORT TopoDS_Shape GetLowerShape();

    // Getter for inner segment face
    TIGL_EXPORT TopoDS_Face GetInnerFace();

    // Getter for outer segment face
    TIGL_EXPORT TopoDS_Face GetOuterFace();

    // Gets a point in relative wing coordinates for a given eta and xsi
    TIGL_EXPORT gp_Pnt GetPoint(double eta, double xsi) const;

    // For eta==0 or eta==1 returns the chordlinePoints, otherwise returns the
    // eta/xsi point in the wing coordinate system (calls GetPoint)
    TIGL_EXPORT gp_Pnt GetMidplaneOrChordlinePoint(double eta, double xsi) const;

    // Getter for leading edge point at the relative position, which must be
    // defined between 0 (inner point on leadinge edge) and 1 (outer point)
    TIGL_EXPORT gp_Pnt GetLeadingEdgePoint(double relativePos) const;

    // Getter for trailing edge point at the relative position, which must be
    // defined between 0 (inner point on trailing edge) and 1 (outer point)
    TIGL_EXPORT gp_Pnt GetTrailingEdgePoint(double relativePos) const;

    // Getter for section element face
    TIGL_EXPORT TopoDS_Face GetSectionElementFace(const std::string& sectionElementUID) const;

    // Getter for eta and xsi for passed point
    // Passed point must be in wing coordinate system
    TIGL_EXPORT void GetMidplaneEtaXsi(const gp_Pnt& p, double& eta, double& xsi) const;

    // Getter for eta direction of midplane (no X-component)
    TIGL_EXPORT gp_Vec GetMidplaneEtaDir(double eta) const;

    // Getter for midplane normal vector
    TIGL_EXPORT gp_Vec GetMidplaneNormal(double eta) const;

    // Get the eta xsi coordinate from a segment point (given by seta, sxsi)
    TIGL_EXPORT void GetEtaXsiFromSegmentEtaXsi(const std::string &segmentUID, double seta, double sxsi, double &eta, double &xsi) const;

    // Gets the volume of this segment
    TIGL_EXPORT double GetVolume();

    // Gets the surface area of this segment
    TIGL_EXPORT double GetSurfaceArea();

    // Returns the segment to a given point on the componentSegment and the nearest point projected onto the loft.
    // Returns null if the point is not an that wing, i.e. deviates more than 1 cm from the wing
    TIGL_EXPORT const CCPACSWingSegment* findSegment(double x, double y, double z, gp_Pnt& nearestPoint, double& deviation) const;

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const { return TIGL_COMPONENT_WINGCOMPSEGMENT | TIGL_COMPONENT_SEGMENT | TIGL_COMPONENT_LOGICAL; }

    TIGL_EXPORT MaterialList GetMaterials(double eta, double xsi, TiglStructureType);

    // returns a list of segments that belong to this component segment
    // TODO: return const-reference to avoid potential harmful modification of wingSegments member
    TIGL_EXPORT SegmentList& GetSegmentList() const;
        
    // creates an (iso) component segment line 
    TIGL_EXPORT TopoDS_Wire GetCSLine(double eta1, double xsi1, double eta2, double xsi2, int NSTEPS=101);
        
    // calculates the intersection of a segment iso eta line with a component segment line (defined by its start and end point)
    // returns the xsi coordinate of the intersection
    TIGL_EXPORT void GetSegmentIntersection(const std::string& segmentUID, double csEta1, double csXsi1, double csEta2, double csXsi2, double eta, double& xsi);

    // Getter for the normalized leading edge direction
    TIGL_EXPORT gp_Vec GetLeadingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID = "") const;

    // Getter for the normalized trailing edge direction
    TIGL_EXPORT gp_Vec GetTrailingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID = "") const;

    // Getter for the length of the leading edge
    TIGL_EXPORT double GetLeadingEdgeLength() const;

    // Getter for the length of the trailing edge
    TIGL_EXPORT double GetTrailingEdgeLength() const;

    // Getter for the midplane line between two eta-xsi points
    TIGL_EXPORT TopoDS_Wire GetMidplaneLine(const gp_Pnt& startPoint, const gp_Pnt& endPoint) const;

    //Return the shape for the componentsegment midplane 
    TIGL_EXPORT TopoDS_Shape GetMidplaneShape() const;

    // Getter for the leading edge line
    TIGL_EXPORT const TopoDS_Wire& GetLeadingEdgeLine() const;

    // Getter for the trailing edge line
    TIGL_EXPORT const TopoDS_Wire& GetTrailingEdgeLine() const;

    // Getter for inner segment UID
    TIGL_EXPORT const std::string& GetInnerSegmentUID() const;

    // Getter for outer segment UID
    TIGL_EXPORT const std::string& GetOuterSegmentUID() const;

    // Method for checking whether segment is contained in componentSegment
    TIGL_EXPORT bool IsSegmentContained(const CCPACSWingSegment& segment) const;

    // Getter for upper and lower shell
    TIGL_EXPORT const CCPACSWingShell& GetUpperShell() const;
    TIGL_EXPORT CCPACSWingShell& GetUpperShell();
    TIGL_EXPORT const CCPACSWingShell& GetLowerShell() const;
    TIGL_EXPORT CCPACSWingShell& GetLowerShell();

    // computes the xsi coordinate on a straight line in global space, given an eta coordinate
    TIGL_EXPORT void InterpolateOnLine(double csEta1, double csXsi1, double csEta2, double csXsi2, double eta, double &xsi, double &errorDistance);
protected:
    // Cleanup routine
    void Cleanup();

    // Update internal segment data
    void Update();

    // Builds the loft between the two segment sections
    PNamedShape BuildLoft();

    // Method for building wires for eta-, leading edge-, trailing edge-lines
    void BuildLines() const;

    // Returns an upper or lower point on the segment surface in
    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
    // edge on the outer wing profile. If fromUpper is true, a point
    // on the upper surface is returned, otherwise from the lower.
//  gp_Pnt GetPoint(double eta, double xsi, bool fromUpper);

private:
    // get short name for loft
    std::string GetShortShapeName();

    std::vector<int> findPath(const std::string& fromUid, const::std::string& toUID, const std::vector<int>& curPath, bool forward) const;

    // Returns the leading edge direction of the segment with the passed UID
    gp_Vec GetLeadingEdgeDirection(const std::string& segmentUID) const;

    // Returns the trailing edge direction of the segment with the passed UID
    gp_Vec GetTrailingEdgeDirection(const std::string& segmentUID) const;

    void UpdateProjectedLeadingEdge() const;
    void UpdateExtendedChordFaces();


private:
    std::string          toElementUID;         /**< Outer segment uid (tip)                 */
    CCPACSWing*          wing;                 /**< Parent wing                             */
    double               myVolume;             /**< Volume of this segment                  */
    double               mySurfaceArea;        /**< Surface area of this segment            */
    TopoDS_Shape         upperShape;           /**< Upper shape of this componentSegment    */
    TopoDS_Shape         lowerShape;           /**< Lower shape of this componentSegment    */
    mutable Handle(Geom_Curve) projLeadingEdge;/**< (Extended) Leading edge projected into y-z plane */
    mutable SegmentList  wingSegments;         /**< List of segments belonging to the component segment */
    TopoDS_Face          innerFace;            /**< [[CAS_AES]] added inner segment face    */
    TopoDS_Face          outerFace;            /**< [[CAS_AES]] added outer segment face    */
    CTiglPointTranslator extendedOuterChord;   /**< Extended outer segment chord face */
    CTiglPointTranslator extendedInnerChord;   /**< Extended inner segment chord face */
    Handle(Geom_Surface) upperSurface;
    Handle(Geom_Surface) lowerSurface;
    bool                 surfacesAreValid;

    mutable TopoDS_Wire  etaLine;                  // 2d version (in YZ plane) of leadingEdgeLine
    mutable TopoDS_Wire  extendedEtaLine;          // 2d version (in YZ plane) of extendedLeadingEdgeLine
    mutable TopoDS_Wire  leadingEdgeLine;          // leading edge as wire
    mutable TopoDS_Wire  extendedLeadingEdgeLine;  // leading edge extended along y-axis, see CPACS spar geometry definition
    mutable TopoDS_Wire  trailingEdgeLine;         // trailing edge as wire
    mutable TopoDS_Wire  extendedTrailingEdgeLine; // trailing edge extended along y-axis, see CPACS spar geometry definition
    mutable bool         linesAreValid;
};

} // end namespace tigl

#endif // CCPACSWINGCOMPONENTSEGMENT_H
